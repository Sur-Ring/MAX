#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/netlink.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <signal.h>
#include "./include/netlink.h"
#include "./include/handler.h"

int skfd;
struct sockaddr_nl saddr, daddr;
static struct nl_sock *sk = NULL;

// 只会在初始化以及收到信息的时候调用
// NLMSG_SPACE里面的值不需要包括包头，它会在计算的时候加上去
static void deamon_send_nl(uint32_t seqn, struct hanlder_rep* reply) {
    int err;
    // 整理包头
    struct nlmsghdr* nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(reply->rep_size));
    memset(nlh, 0, sizeof(struct nlmsghdr));
    nlh->nlmsg_len = NLMSG_SPACE(reply->rep_size);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = seqn;
    nlh->nlmsg_pid = saddr.nl_pid; //self port
    // 装填返回值
    struct cmd_rep* data = (struct cmd_rep*)NLMSG_DATA(nlh);
    PR("添加总长度%ld的返回值",reply->rep_size);
    memcpy(data, reply, reply->rep_size);
    if(reply->ret!=0){
        PRE("失败:%d",reply->ret);
    }
    // 发送
    err = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if(!err){
        PRE("sendto error");
    }
    free((void *)nlh);
}

typedef struct _user_msg_info{
    struct nlmsghdr hdr;
    char  msg[1024*1024];
} user_msg_info;

void recv_nl() {
    user_msg_info u_info;
    memset(&u_info, 0, sizeof(u_info));
    socklen_t len = sizeof(struct sockaddr_nl);
    int err = recvfrom(skfd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
    if(!err){
        perror("recv form kernel error\n");
    }

    uint32_t seq = u_info.hdr.nlmsg_seq;
    PR("收到信息序号为:%u", seq);

    struct cmd_req* data = (struct cmd_req*)u_info.msg;
    struct hanlder_rep* reply = handle_cmd(data);

    PR("信息 %u 处理完成，开始回复",seq);
    deamon_send_nl(seq, reply);
    free(reply);
    PR("信息 %u 回复完成\n", seq);
}

static void deamon_send_init_nl() {
    int err;
    // 整理包头
    struct nlmsghdr* nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(0));
    memset(nlh, 0, sizeof(struct nlmsghdr));
    nlh->nlmsg_len = NLMSG_SPACE(0);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = saddr.nl_pid; //self port
    // 发送
    err = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if(!err){
        PRE("sendto error");
    }
    free((void *)nlh);
}

int init_nl() {
    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER); //Create a socket using user defined protocol NETLINK_TEST.
    if(skfd == -1){
        perror("create socket error\n");
        return -1;
    }

    //Source address.
    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK; //AF_NETLINK
    saddr.nl_pid = getpid();
    saddr.nl_groups = 0;
    if(bind(skfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0){
        perror("bind() error\n");
        close(skfd);
        return -1;
    }

    //Destination address.
    memset(&daddr, 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK;
    daddr.nl_pid = 0;    // to kernel 
    daddr.nl_groups = 0;

    //ping so kernel can get our pid
    PRI("连接到kapi");
    deamon_send_init_nl();
    PRI("Netlink connected, message sent to kernel");

    return 0;
}

int exit_nl() {
    nl_socket_free(sk);
    return 0;
}