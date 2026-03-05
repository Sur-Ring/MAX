#ifndef __MAX_API_KERNEL_H__
#define __MAX_API_KERNEL_H__

#include "hsa_def.h"

typedef void* address_t;
typedef struct {
  address_t kernel;
  address_t user;
} address_pair;
typedef struct{
  uint64_t kernel_object;
  uint64_t kernarg_segment_size;
  uint64_t group_segment_size;
  uint64_t private_segment_size;
}kernel_info;

struct map_node;

typedef struct MAX_signal{
  hsa_signal_t user_signal;
  hsa_signal_t kern_signal;
  uint64_t* doorbell_ptr;
  struct map_node* signal_node;
}MAX_signal;

typedef struct MAX_queue{
  hsa_queue_t* user_queue;
  hsa_queue_t* kern_queue;
  void* kern_ringbuffer;
  uint64_t* wptr;
  uint64_t* rptr;
  struct map_node* queue_node;
  struct map_node* ring_buffer_node;
  MAX_signal doorbell_signal;
  // struct map_node* signal_node;
  // void* doorbell_signal;
  // uint64_t* hardware_doorbell;
}MAX_queue;


extern hsa_status_t MAX_empty(void);
extern hsa_status_t MAX_init(void);
extern hsa_status_t MAX_get_gpu_agent(hsa_agent_t* agent);
extern hsa_status_t MAX_get_kernarg_region(hsa_agent_t agent,hsa_region_t* kernarg_region);
extern hsa_status_t MAX_get_device_pool(hsa_agent_t agent,hsa_amd_memory_pool_t* device_pool);
extern hsa_status_t MAX_signal_create(hsa_signal_value_t initial_value, MAX_signal* signal);
extern hsa_status_t MAX_queue_create(hsa_agent_t agent, uint32_t size, hsa_queue_type32_t type, MAX_queue* queue);
extern hsa_status_t MAX_load_module(hsa_agent_t agent, char* module_name, hsa_executable_t* executable);
extern hsa_status_t MAX_load_kernel(hsa_agent_t agent, hsa_executable_t executable, char* kernel_name, kernel_info* kernel);
extern hsa_status_t MAX_launch_kernel_with_signal(int use_kenerl_path, MAX_queue queue, uint16_t header, uint16_t setup, 
    uint16_t workgroup_size_x, uint16_t workgroup_size_y, uint16_t workgroup_size_z,
    uint32_t grid_size_x, uint32_t grid_size_y, uint32_t grid_size_z,
    uint64_t kernel_object, MAX_signal completion_signal, void* args
);
extern hsa_status_t MAX_launch_kernel(int use_kenerl_path, MAX_queue queue, uint16_t header, uint16_t setup, 
  uint16_t workgroup_size_x, uint16_t workgroup_size_y, uint16_t workgroup_size_z,
  uint32_t grid_size_x, uint32_t grid_size_y, uint32_t grid_size_z,
  uint64_t kernel_object, void* args
);

extern hsa_status_t MAX_alloc(size_t size, address_pair* addr_pair);
extern hsa_status_t MAX_free(address_pair addr_pair);

extern hsa_status_t MAX_pool_alloc(hsa_amd_memory_pool_t pool, size_t size, address_pair* addr_pair);
extern hsa_status_t MAX_pool_free(address_pair addr_pair);

extern hsa_status_t MAX_region_alloc(hsa_region_t region, size_t size, address_pair* addr_pair);
extern hsa_status_t MAX_region_free(address_pair addr_pair);

extern hsa_status_t MAX_copy(hsa_agent_t agent, address_t dst_user, address_t src_user, size_t size, MAX_signal completion_signal);

extern uint64_t MAX_queue_add_write_index_relaxed(MAX_queue queue, hsa_signal_value_t value);
extern hsa_status_t MAX_signal_store_relaxed(MAX_signal signal, hsa_signal_value_t value);
extern hsa_signal_value_t MAX_signal_wait_acquire(MAX_signal signal, hsa_signal_condition_t condition, hsa_signal_value_t compare_value, uint64_t timeout_hint, hsa_wait_state_t wait_state_hint);

#endif  // header guard
