#ifndef __MAX_NETLINK_H__
#define __MAX_NETLINK_H__

#include "../../common/def.h"
#include "../../common/api_kernel.h"
#include "../../common/cmd.h"
#include <linux/completion.h>

// 保存在收到返回的数据包时所需的数据
struct cmd_entry {
    struct completion done; //是否完成
    char sync; // 是否同步
    size_t rep_size;
    struct cmd_rep* reply;
};

// 地址转换要用到pid
extern pid_t deamon_pid;

int netlink_init(void);
int netlink_exit(void);
void netlink_send(int sync, struct cmd_req* request, size_t req_size, struct cmd_rep* reply, size_t rep_size);

#endif