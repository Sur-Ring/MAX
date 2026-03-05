#include <linux/slab.h>
#include <linux/io.h>
#include "../../common/cmd.h"
#include "../../common/api_kernel.h"
#include "netlink.h"
#include "shm.h"

// 装填普通参数
#define pack_arg(arg_p, arg) {memcpy((arg_p),&(arg),sizeof(arg));(arg_p)+=sizeof(arg);}
// 装填字符串参数
#define pack_arg_str(arg_p, arg_str) {strcpy((arg_p),(arg_str));(arg_p)+=strlen(arg_str)+1;}
// 装填内存参数
#define pack_arg_mem(arg_p, arg_mem, arg_size) {memcpy((arg_p), (arg_mem), (arg_size));(arg_p)+=(arg_size);}

hsa_status_t MAX_empty(){
    // 准备返回包
    struct cmd_rep_empty reply;
    // 准备请求包
    struct cmd_req_empty* request = kmalloc(sizeof(struct cmd_req_empty), GFP_KERNEL);
    // 装填指令
    request->api_id = MAXAPI_empty;
    // 装填额外参数
    // 发送
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包
    kfree(request);
    // 处理返回包
	return reply.ret;
}EXPORT_SYMBOL(MAX_empty);

hsa_status_t MAX_init(){
    // 准备返回包
    struct cmd_rep_init reply;
    // 准备请求包
    struct cmd_req_init* request = kmalloc(sizeof(struct cmd_req_init), GFP_KERNEL);
    // 装填指令
    request->api_id = MAXAPI_init;
    // 装填额外参数
    // 发送
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包
    kfree(request);
    // 处理返回包
	return reply.ret;
}EXPORT_SYMBOL(MAX_init);

hsa_status_t MAX_get_gpu_agent(hsa_agent_t* gpu_agent){
    // 准备返回包
    struct cmd_rep_get_gpu_agent reply;
    // 准备请求包
    struct cmd_req_get_gpu_agent* request = kmalloc(sizeof(struct cmd_req_get_gpu_agent), GFP_KERNEL);
    // 装填指令
    request->api_id = MAXAPI_get_gpu_agent;
    // 装填额外参数
    // 发送
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包
    kfree(request);
    // 处理返回包
    *gpu_agent = reply.gpu_agent;
    PR("返回 agent:0x%llx",gpu_agent->handle);
	return reply.ret;
}EXPORT_SYMBOL(MAX_get_gpu_agent);

hsa_status_t MAX_get_kernarg_region(hsa_agent_t gpu_agent, hsa_region_t* kernarg_region){
    // 准备返回包-改为对应包
    struct cmd_rep_get_kernarg_region reply;
    // 准备请求包-修改长度
    struct cmd_req_get_kernarg_region* request = kmalloc(sizeof(struct cmd_req_get_kernarg_region), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_get_kernarg_region;
    // 装填额外参数-根据参数修改
    request->gpu_agent = gpu_agent;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    *kernarg_region = reply.kernarg_region;
	return reply.ret;
}EXPORT_SYMBOL(MAX_get_kernarg_region);

hsa_status_t MAX_get_device_pool(hsa_agent_t gpu_agent, hsa_amd_memory_pool_t* device_pool){
    // 准备返回包-改为对应包
    struct cmd_rep_get_device_pool reply;
    // 准备请求包-修改长度
    struct cmd_req_get_device_pool* request = kmalloc(sizeof(struct cmd_req_get_device_pool), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_get_device_pool;
    // 装填额外参数-根据参数修改
    request->gpu_agent = gpu_agent;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    *device_pool = reply.device_pool;
	return reply.ret;
}EXPORT_SYMBOL(MAX_get_device_pool);

hsa_status_t MAX_signal_create(hsa_signal_value_t initial_value, MAX_signal* signal){
    // 准备返回包-改为对应包
    struct cmd_rep_signal_create reply;
    // 准备请求包-修改长度
    struct cmd_req_signal_create* request = kmalloc(sizeof(struct cmd_req_signal_create), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_signal_create;
    // 装填额外参数-根据参数修改
    request->initial_value = initial_value;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    // 重映射signal
    remap_signal(reply.signal, signal);
	return reply.ret;
}EXPORT_SYMBOL(MAX_signal_create);

hsa_status_t MAX_queue_create(hsa_agent_t agent, uint32_t size, hsa_queue_type32_t type, MAX_queue* queue){
    // 准备返回包-改为对应包
    struct cmd_rep_queue_create reply;
    // 准备请求包-修改长度
    struct cmd_req_queue_create* request = kmalloc(sizeof(struct cmd_req_queue_create), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_queue_create;
    // 装填额外参数-根据参数修改
    if(size%0x40)size+=0x40-size%0x40;
    request->agent = agent;
    request->size = size;
    request->type = type;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    // 映射queue
    remap_queue(reply.queue, queue);
	return reply.ret;
}EXPORT_SYMBOL(MAX_queue_create);

hsa_status_t MAX_load_module(hsa_agent_t agent, char* module_path, hsa_executable_t* executable){
    // 准备返回包-改为对应包
    struct cmd_rep_load_module reply;
    // 准备请求包-修改长度
    size_t module_path_len = strlen(module_path);
    struct cmd_req_load_module* request = kmalloc(sizeof(struct cmd_req_load_module)+module_path_len+1, GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_load_module;
    // 装填额外参数-根据参数修改
    request->agent = agent;
    memcpy(request->module_path, module_path, module_path_len+1);
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request)+module_path_len+1, (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    *executable = reply.executable;
	return reply.ret;
}EXPORT_SYMBOL(MAX_load_module);

hsa_status_t MAX_load_kernel(hsa_agent_t agent, hsa_executable_t executable, char* kernel_name, kernel_info* kernel){
    // 准备返回包-改为对应包
    struct cmd_rep_load_kernel reply;
    // 准备请求包-修改长度
    size_t kernel_name_len = strlen(kernel_name);
    struct cmd_req_load_kernel* request = kmalloc(sizeof(struct cmd_req_load_kernel)+kernel_name_len+1, GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_load_kernel;
    // 装填额外参数-根据参数修改
    request->agent = agent;
    request->executable = executable;
    memcpy(request->kernel_name, kernel_name, kernel_name_len+1);
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request)+kernel_name_len+1, (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    kernel->kernel_object = reply.kernel_object;
    kernel->kernarg_segment_size = reply.kernarg_segment_size;
    kernel->group_segment_size = reply.group_segment_size;
    kernel->private_segment_size = reply.private_segment_size;
	return reply.ret;
}EXPORT_SYMBOL(MAX_load_kernel);

hsa_status_t MAX_launch_kernel_with_signal(int use_kenerl_path, MAX_queue queue, uint16_t header, uint16_t setup, 
        uint16_t workgroup_size_x, uint16_t workgroup_size_y, uint16_t workgroup_size_z,
        uint32_t grid_size_x, uint32_t grid_size_y, uint32_t grid_size_z,
        uint64_t kernel_object, MAX_signal completion_signal, void* args)
{
    // 准备返回包-改为对应包
    struct cmd_rep_launch_kernel reply;
    // 准备请求包-修改长度
    struct cmd_req_launch_kernel* request = kmalloc(sizeof(struct cmd_req_launch_kernel), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_launch_kernel_with_signal;
    // 装填额外参数-根据参数修改
    request->queue = queue.user_queue;
    request->header = header;
    request->setup = setup;
    request->workgroup_size_x = workgroup_size_x;
    request->workgroup_size_y = workgroup_size_y;
    request->workgroup_size_z = workgroup_size_z;
    request->grid_size_x = grid_size_x;
    request->grid_size_y = grid_size_y;
    request->grid_size_z = grid_size_z;
    request->kernel_object = kernel_object;
    request->completion_signal = completion_signal.user_signal;
    request->args = args;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.ret;
}EXPORT_SYMBOL(MAX_launch_kernel_with_signal);

hsa_status_t MAX_launch_kernel(int use_kenerl_path, MAX_queue queue, uint16_t header, uint16_t setup, 
    uint16_t workgroup_size_x, uint16_t workgroup_size_y, uint16_t workgroup_size_z,
    uint32_t grid_size_x, uint32_t grid_size_y, uint32_t grid_size_z,
    uint64_t kernel_object, void* args)
{
    // 准备返回包-改为对应包
    struct cmd_rep_launch_kernel reply;
    // 准备请求包-修改长度
    struct cmd_req_launch_kernel* request = kmalloc(sizeof(struct cmd_req_launch_kernel), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_launch_kernel;
    // 装填额外参数-根据参数修改
    request->queue = queue.user_queue;
    request->header = header;
    request->setup = setup;
    request->workgroup_size_x = workgroup_size_x;
    request->workgroup_size_y = workgroup_size_y;
    request->workgroup_size_z = workgroup_size_z;
    request->grid_size_x = grid_size_x;
    request->grid_size_y = grid_size_y;
    request->grid_size_z = grid_size_z;
    request->kernel_object = kernel_object;
    request->args = args;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.ret;
}EXPORT_SYMBOL(MAX_launch_kernel);

hsa_status_t MAX_alloc(size_t size, address_pair* addr_pair){
    // 准备返回包-改为对应包
    struct cmd_rep_alloc reply;
    // 准备请求包-修改长度
    struct cmd_req_alloc* request = kmalloc(sizeof(struct cmd_req_alloc), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_alloc;
    // 装填额外参数-根据参数修改
    if(size%PAGE_SIZE){
        size=(size|(PAGE_SIZE-1))+1;
    }
    request->size = size;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    addr_pair->user = reply.addr;
    // 重映射 kern_addr
    addr_pair->kernel = remap_addr(addr_pair->user, size);
	return reply.ret;
}EXPORT_SYMBOL(MAX_alloc);

hsa_status_t MAX_free(address_pair addr_pair){
    // 解除映射 kern_addr
    unremap_addr(addr_pair.kernel);
    // 准备返回包-改为对应包
    struct cmd_rep_free reply;
    // 准备请求包-修改长度
    struct cmd_req_free* request = kmalloc(sizeof(struct cmd_req_free), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_free;
    // 装填额外参数-根据参数修改
    request->addr = addr_pair.user;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.ret;
}EXPORT_SYMBOL(MAX_free);

//
hsa_status_t MAX_pool_alloc(hsa_amd_memory_pool_t pool, size_t size, address_pair* addr_pair){
    // 准备返回包-改为对应包
    struct cmd_rep_pool_alloc reply;
    // 准备请求包-修改长度
    struct cmd_req_pool_alloc* request = kmalloc(sizeof(struct cmd_req_pool_alloc), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_pool_alloc;
    // 装填额外参数-根据参数修改
    if(size%PAGE_SIZE){
        size=(size|(PAGE_SIZE-1))+1;
    }
    request->size = size;
    request->pool = pool;
    // PR("\t\t\t size=%d\n", size);
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    addr_pair->user = reply.addr;
    addr_pair->kernel = 0;
    // 重映射 kern_addr
    // *kern_addr = remap_addr(*user_addr, size);
	return reply.ret;
}EXPORT_SYMBOL(MAX_pool_alloc);

hsa_status_t MAX_pool_free(address_pair addr_pair){
    // 解除映射 kern_addr
    // unremap_addr(kern_addr);
    // 准备返回包-改为对应包
    struct cmd_rep_pool_free reply;
    // 准备请求包-修改长度
    struct cmd_req_pool_free* request = kmalloc(sizeof(struct cmd_req_pool_free), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_pool_free;
    // 装填额外参数-根据参数修改
    request->addr = addr_pair.user;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.ret;
}EXPORT_SYMBOL(MAX_pool_free);

hsa_status_t MAX_region_alloc(hsa_region_t region, size_t size, address_pair* addr_pair){
    // 准备返回包-改为对应包
    struct cmd_rep_region_alloc reply;
    // 准备请求包-修改长度
    struct cmd_req_region_alloc* request = kmalloc(sizeof(struct cmd_req_region_alloc), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_region_alloc;
    // 装填额外参数-根据参数修改
    if(size%PAGE_SIZE){
        size=(size|(PAGE_SIZE-1))+1;
    }
    request->size = size;
    request->region = region;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    addr_pair->user = reply.addr;
    // 重映射 kern_addr
    addr_pair->kernel = remap_addr(addr_pair->user, size);
	return reply.ret;
}EXPORT_SYMBOL(MAX_region_alloc);

hsa_status_t MAX_region_free(address_pair addr_pair){
    // 解除映射 kern_addr
    unremap_addr(addr_pair.kernel);
    // 准备返回包-改为对应包
    struct cmd_rep_region_free reply;
    // 准备请求包-修改长度
    struct cmd_req_region_free* request = kmalloc(sizeof(struct cmd_req_region_free), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_region_free;
    // 装填额外参数-根据参数修改
    request->addr = addr_pair.user;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.ret;
}EXPORT_SYMBOL(MAX_region_free);
//

hsa_status_t MAX_copy(hsa_agent_t agent, address_t dst_user, address_t src_user, size_t size, MAX_signal completion_signal){
    // 准备返回包-改为对应包
    struct cmd_rep_copy reply;
    // 准备请求包-修改长度
    struct cmd_req_copy* request = kmalloc(sizeof(struct cmd_req_copy), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_copy;
    // 装填额外参数-根据参数修改
    request->gpu_agent = agent;
    request->dst = dst_user;
    request->src = src_user;
    request->size = size;
    request->completion_signal = completion_signal.user_signal;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.ret;
}EXPORT_SYMBOL(MAX_copy);

extern uint64_t MAX_queue_add_write_index_relaxed(MAX_queue queue, hsa_signal_value_t value){
    // 准备返回包-改为对应包
    struct cmd_rep_queue_add_write_index_relaxed reply;
    // 准备请求包-修改长度
    struct cmd_req_queue_add_write_index_relaxed* request = kmalloc(sizeof(struct cmd_req_queue_add_write_index_relaxed), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_queue_add_write_index_relaxed;
    // 装填额外参数-根据参数修改
    request->queue = queue.user_queue;
    request->value = value;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.index;
}EXPORT_SYMBOL(MAX_queue_add_write_index_relaxed);

extern hsa_status_t MAX_signal_store_relaxed(MAX_signal signal, hsa_signal_value_t value){
    // 准备返回包-改为对应包
    struct cmd_rep_signal_store_relaxed reply;
    // 准备请求包-修改长度
    struct cmd_req_signal_store_relaxed* request = kmalloc(sizeof(struct cmd_req_signal_store_relaxed), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_signal_store_relaxed;
    // 装填额外参数-根据参数修改
    request->signal = signal.user_signal;
    request->value = value;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.ret;
}EXPORT_SYMBOL(MAX_signal_store_relaxed);

extern hsa_signal_value_t MAX_signal_wait_acquire(MAX_signal signal, hsa_signal_condition_t condition, hsa_signal_value_t compare_value, uint64_t timeout_hint, hsa_wait_state_t wait_state_hint){
    // 准备返回包-改为对应包
    struct cmd_rep_signal_wait_acquire reply;
    // 准备请求包-修改长度
    struct cmd_req_signal_wait_acquire* request = kmalloc(sizeof(struct cmd_req_signal_wait_acquire), GFP_KERNEL);
    // 装填指令-修改指令ID
    request->api_id = MAXAPI_signal_wait_acquire;
    // 装填额外参数-根据参数修改
    request->signal = signal.user_signal;
    request->condition = condition;
    request->compare_value = compare_value;
    request->timeout_hint = timeout_hint;
    request->wait_state_hint = wait_state_hint;
    // 发送-无需修改
    netlink_send(API_SYNC, (struct cmd_req*)request, sizeof(*request), (struct cmd_rep*)&reply, sizeof(reply));
    // 释放请求包-无需修改
    kfree(request);
    // 处理返回包-根据返回值修改
    return reply.complete;
}EXPORT_SYMBOL(MAX_signal_wait_acquire);
