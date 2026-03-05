#ifndef __HANDLER_H__
#define __HANDLER_H__

#include "../../common/cmd.h"

// hanlder 使用的返回包
// rep_size是整个cmd_rep的大小
struct hanlder_rep{
    size_t rep_size;
    hsa_status_t ret;
    char add_args[0];
};

struct hanlder_rep* handle_cmd(struct cmd_req* data);

#endif