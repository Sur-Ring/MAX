#ifndef __MAX_CMD_H__
#define __MAX_CMD_H__

#ifdef __MAX_KERNEL__
#include "hsa_def.h"
#else
#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#endif

typedef unsigned int cmd_idx;

#define API_ASYNC 0
#define API_SYNC  1

typedef enum MAX_api_id {
    MAXAPI_empty = 0,
    MAXAPI_init = 1,
    MAXAPI_get_gpu_agent,
    MAXAPI_get_kernarg_region,
    MAXAPI_get_device_pool,
    MAXAPI_signal_create,
    MAXAPI_queue_create,
    MAXAPI_load_module,
    MAXAPI_load_kernel,
    MAXAPI_launch_kernel,
    MAXAPI_launch_kernel_with_signal,
    MAXAPI_alloc,
    MAXAPI_free,
    MAXAPI_region_alloc,
    MAXAPI_region_free,
    MAXAPI_pool_alloc,
    MAXAPI_pool_free,
    MAXAPI_copy,
    MAXAPI_queue_add_write_index_relaxed,
    MAXAPI_signal_store_relaxed,
    MAXAPI_signal_wait_acquire,
}MAX_API;

#define REQ_PACK(name, ...) \
    struct cmd_req_##name { \
        MAX_API api_id; \
        __VA_ARGS__ \
    };

#define REP_PACK(name, ...) \
    struct cmd_rep_##name { \
        size_t size; \
        hsa_status_t ret; \
        __VA_ARGS__ \
    };

struct cmd_req{
    MAX_API api_id;
    char arg[0];
};
struct cmd_rep{
    size_t size; // 返回值长度
    hsa_status_t ret; // hsa返回值
    char res[0]; // 额外返回值
};

// 空命令结构体
REQ_PACK(empty);
REP_PACK(empty);

// 初始化命令结构体
REQ_PACK(init);
REP_PACK(init);

// 获取GPU代理结构体
REQ_PACK(get_gpu_agent,
    char arg[0]; // 柔性数组
);
REP_PACK(get_gpu_agent,
    hsa_agent_t gpu_agent; // 额外返回值
);

// 获取内核参数区域结构体
REQ_PACK(get_kernarg_region,
    hsa_agent_t gpu_agent;
);
REP_PACK(get_kernarg_region,
    hsa_region_t kernarg_region; // 额外返回值
);

// 获取设备内存池结构体
REQ_PACK(get_device_pool,
    hsa_agent_t gpu_agent;
);
REP_PACK(get_device_pool,
    hsa_amd_memory_pool_t device_pool; // 额外返回值
);

REQ_PACK(signal_create,
    hsa_signal_value_t initial_value;
);
REP_PACK(signal_create,
    hsa_signal_t signal;    /* 额外返回值 */
);

REQ_PACK(queue_create,
    hsa_agent_t agent;
    uint32_t size;
    hsa_queue_type32_t type;
);
REP_PACK(queue_create,
    hsa_queue_t* queue;     /* 额外返回值 */
);

REQ_PACK(load_module,
    hsa_agent_t agent;
    char module_path[0];    // 柔性数组
);
REP_PACK(load_module,
    hsa_executable_t executable; /* 额外返回值 */
);
// 使用宏定义结构体
REQ_PACK(load_kernel,
    hsa_agent_t agent;
    hsa_executable_t executable;
    char kernel_name[0];
);
REP_PACK(load_kernel,
    uint64_t kernel_object; // 额外返回值
    uint64_t kernarg_segment_size; // 额外返回值
    uint64_t group_segment_size; // 额外返回值
    uint64_t private_segment_size; // 额外返回值
);

REQ_PACK(launch_kernel,
    hsa_queue_t *queue;
    uint16_t header;
    uint16_t setup;
    uint16_t workgroup_size_x;
    uint16_t workgroup_size_y;
    uint16_t workgroup_size_z;
    uint32_t grid_size_x;
    uint32_t grid_size_y;
    uint32_t grid_size_z;
    uint64_t kernel_object;
    hsa_signal_t completion_signal;
    void* args;
);
REP_PACK(launch_kernel);

REQ_PACK(alloc,
    size_t size; // hsa返回值
);
REP_PACK(alloc,
    void* addr; // hsa返回值
);

REQ_PACK(free,
    void* addr; // hsa返回值
);
REP_PACK(free);

REQ_PACK(pool_alloc,
    size_t size; // hsa返回值
    hsa_amd_memory_pool_t pool;
);
REP_PACK(pool_alloc,
    void* addr; // hsa返回值
);

REQ_PACK(pool_free,
    void* addr; // hsa返回值
);
REP_PACK(pool_free);

REQ_PACK(region_alloc,
    size_t size; // hsa返回值
    hsa_region_t region;
);
REP_PACK(region_alloc,
    void* addr; // hsa返回值
);

REQ_PACK(region_free,
    void* addr; // hsa返回值
);
REP_PACK(region_free);

REQ_PACK(copy,
    hsa_agent_t gpu_agent; // hsa返回值
    void* dst;
    void* src;
    size_t size;
    hsa_signal_t completion_signal;
);
REP_PACK(copy);

REQ_PACK(queue_add_write_index_relaxed,
    hsa_queue_t *queue;
    uint64_t value;
);
REP_PACK(queue_add_write_index_relaxed,
    uint64_t index; // 额外返回值
);

REQ_PACK(signal_store_relaxed,
    hsa_signal_t signal;
    hsa_signal_value_t value;
);
REP_PACK(signal_store_relaxed);

REQ_PACK(signal_wait_acquire,
    hsa_signal_t signal;
    hsa_signal_condition_t condition;
    hsa_signal_value_t compare_value;
    uint64_t timeout_hint;
    hsa_wait_state_t wait_state_hint;
);
REP_PACK(signal_wait_acquire,
    hsa_signal_value_t complete;
);
//*/

#endif