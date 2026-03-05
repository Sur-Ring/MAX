#include "netlink.h"
#include "shm.h"

#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

struct sock *nl_sk = NULL;
pid_t deamon_pid = -1;

static DEFINE_SPINLOCK(counter_lock);
cmd_idx cmd_counter = 1;
static cmd_idx max_counter = (1<<10);

DEFINE_XARRAY(cmd_xa); 
static struct kmem_cache *cmd_cache;

static struct cmd_entry* load_cmd_entry(cmd_idx xa_idx){
    struct cmd_entry* cmd_entry;
    xa_lock(&cmd_xa);
    cmd_entry = (struct cmd_entry*) xa_load(&cmd_xa, xa_idx);
    xa_unlock(&cmd_xa);
    if (!cmd_entry) {
        PRE("Error (0) looking up task %u at xarray", xa_idx);
        xa_erase(&cmd_xa, xa_idx);
        return NULL;
    }
    return cmd_entry;
}

static struct cmd_entry* create_cmd_entry(void){
    spin_lock(&counter_lock);
    struct cmd_entry* cmd_entry = (struct cmd_entry*) kmem_cache_alloc(cmd_cache, GFP_KERNEL);
    spin_unlock(&counter_lock);
    if(IS_ERR(cmd_entry)) {
        pr_warn("Error allocating from cache: %ld\n", PTR_ERR(cmd_entry));
        return NULL;
    }
    // 构造完成标记
    init_completion(&cmd_entry->done);
    return cmd_entry;
}

static cmd_idx record_cmd(struct cmd_entry* cmd_entry){
    spin_lock(&counter_lock);
    cmd_idx xa_idx = cmd_counter++;
    if(unlikely(xa_idx >= max_counter))
        cmd_counter = 1;
    spin_unlock(&counter_lock);
    
    if(xa_err(xa_store(&cmd_xa, xa_idx, (void*)cmd_entry, GFP_KERNEL))) {
        pr_warn("Error allocating xa_alloc\n");
    }

    return xa_idx;
}

void change_deamon(u32 new_deamon_pid){
    deamon_pid = new_deamon_pid;
    PRI("Setting deamon PID to %d", deamon_pid);
    shm_change_deamon(deamon_pid);
}

// 收到消息的处理函数
static void netlink_recv(struct sk_buff *skb){
    struct nlmsghdr *nlh = (struct nlmsghdr*) skb->data;
    cmd_idx xa_idx = nlh->nlmsg_seq;

    // 使用序号0来切换守护进程
    if (unlikely(xa_idx == 0)) {
        change_deamon(nlh->nlmsg_pid);
        return;
    }

    // 找到对应的指令
    struct cmd_entry* cmd_entry = load_cmd_entry(xa_idx);
    if (!cmd_entry) {return;}

    // 拷贝返回值
    struct cmd_rep* data = nlmsg_data(nlh);
    if(data->size != cmd_entry->rep_size){
        PRE("返回值长度错误 %d!=%d",data->size, cmd_entry->rep_size);
    }
    memcpy(cmd_entry->reply, data, cmd_entry->rep_size);
    
    // 标记完成
    if (cmd_entry->sync) {
        //if the cmd is sync, whoever we woke up will clean up
        complete(&cmd_entry->done);
    } else {
        //if the cmd is async, no one will read this cmd, so clear
        xa_erase(&cmd_xa, xa_idx);
        kmem_cache_free(cmd_cache, cmd_entry);
    }
}

/* 这个函数用于将处理好的负载做成消息发送出去
xa_idx:消息序号，用于重新找出消息
data:消息负载，包括指令号和更多参数
size:负载大小 
*/
static void _send(int xa_idx, struct cmd_req* data, size_t size){
    // 构造包头，负载大小为int指令加参数包
    struct sk_buff* skb_out = nlmsg_new(size, 0);
    if (!skb_out) {
        PRE("Failed to allocate new skb");
        return;
    }

    // 填充数据包
    struct nlmsghdr* nlh = nlmsg_put(skb_out, 0, xa_idx, NETLINK_REQ, size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    memcpy(nlmsg_data(nlh), data, size);

    // 发送netlink包
    PR("发送信息序号为:%d",xa_idx);
    int err = netlink_unicast(nl_sk, skb_out, deamon_pid, 0);
    if (err < 0) {
        pr_err("Failed to send netlink skb to API server, error=%d\n", err);
        nlmsg_free(skb_out);
    }
}

void netlink_send(int sync, struct cmd_req* request, size_t req_size, struct cmd_rep* reply, size_t rep_size){
    // 创建指令
    struct cmd_entry* cmd_entry = create_cmd_entry();
    // task结构中直接使用提供的返回值指针，所以不需要额外拷贝
    cmd_entry->reply = reply;
    cmd_entry->rep_size = rep_size;
    // 记录是否等待返回
    cmd_entry->sync = sync;
    // 记录任务并分配编号，LAKE这种循环编号在简单情况下够用了
    cmd_idx xa_idx = record_cmd(cmd_entry);

    _send(xa_idx, request, req_size);

    //等待任务完成
    if (sync == API_SYNC) {
        while (1) {
            int err = wait_for_completion_interruptible(&cmd_entry->done);
            if (err == 0) break;
        }
        kmem_cache_free(cmd_cache, cmd_entry);
        xa_erase(&cmd_xa, xa_idx);
    }
}
EXPORT_SYMBOL(netlink_send);

static void null_constructor(void *argument) {}

int netlink_init(void){
    struct netlink_kernel_cfg cfg = {
        .input = netlink_recv,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        PRE("Error creating socket.");
        return -10;
    }

    cmd_cache = kmem_cache_create("MAX_cmd_cache", sizeof(struct cmd_entry), 8, 0, null_constructor);
    if(IS_ERR(cmd_cache)) {
        pr_warn("Error creating cache: %ld\n", PTR_ERR(cmd_cache));
        return -ENOMEM;
    }

    return 0;
}

int netlink_exit(void){
    netlink_kernel_release(nl_sk);
    return 0;
}