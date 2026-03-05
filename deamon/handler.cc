#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "./include/handler.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <map>

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#include "../common/def.h"
hsa_agent_t test_gpu_agent;

static struct hanlder_rep* handle_empty(struct cmd_req* data){
	struct cmd_rep_empty* cmd_rep = (struct cmd_rep_empty*)malloc(sizeof(struct cmd_rep_empty));
	cmd_rep->size = sizeof(*cmd_rep);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
	return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_init(struct cmd_req* data){
	PR("指令: handle_init");
	struct cmd_rep_init* cmd_rep = (struct cmd_rep_init*)malloc(sizeof(struct cmd_rep_init));
	cmd_rep->size = sizeof(*cmd_rep);
	cmd_rep->ret = (hsa_status_t)hsa_init();
	return (struct hanlder_rep*)cmd_rep;
}
static struct hanlder_rep* handle_get_gpu_agent(struct cmd_req* data){
	PR("指令: handle_get_gpu_agent");
	struct cmd_rep_get_gpu_agent* cmd_rep = (struct cmd_rep_get_gpu_agent*)malloc(sizeof(struct cmd_rep_get_gpu_agent));
	cmd_rep->size = sizeof(*cmd_rep);
	cmd_rep->ret = hsa_iterate_agents([](hsa_agent_t agent, void* data) {
		hsa_device_type_t type;
		hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &type);
		if (type == HSA_DEVICE_TYPE_GPU) {
		  *((hsa_agent_t*)data) = agent;
		  return HSA_STATUS_INFO_BREAK;
		}
		return HSA_STATUS_SUCCESS;
	}, &cmd_rep->gpu_agent);
	if(cmd_rep->ret == HSA_STATUS_INFO_BREAK)cmd_rep->ret = HSA_STATUS_SUCCESS;
	PR("分配 agent:%p",cmd_rep->gpu_agent.handle);
	test_gpu_agent = cmd_rep->gpu_agent;
    return (struct hanlder_rep*)cmd_rep;
}
static struct hanlder_rep* handle_get_kernarg_region(struct cmd_req* data){
	PR("指令: handle_get_kernarg_region");
	struct cmd_req_get_kernarg_region* _data = (struct cmd_req_get_kernarg_region*)data;
	struct cmd_rep_get_kernarg_region* cmd_rep = (struct cmd_rep_get_kernarg_region*)malloc(sizeof(struct cmd_rep_get_kernarg_region));
	cmd_rep->size = sizeof(*cmd_rep);
	cmd_rep->ret = hsa_agent_iterate_regions(_data->gpu_agent, [](hsa_region_t region, void* data) {
		hsa_region_segment_t segment;
		hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment);
		if (HSA_REGION_SEGMENT_GLOBAL != segment) {
			return HSA_STATUS_SUCCESS;
		}
	
		hsa_region_global_flag_t flags;
		hsa_region_get_info(region, HSA_REGION_INFO_GLOBAL_FLAGS, &flags);
		if (flags & HSA_REGION_GLOBAL_FLAG_KERNARG) {
			hsa_region_t* ret = (hsa_region_t*) data;
			*ret = region;
			return HSA_STATUS_INFO_BREAK;
		}
	
		return HSA_STATUS_SUCCESS;
	}, &cmd_rep->kernarg_region);
	if(cmd_rep->ret == HSA_STATUS_INFO_BREAK)cmd_rep->ret = HSA_STATUS_SUCCESS;

    return (struct hanlder_rep*)cmd_rep;
}
static struct hanlder_rep* handle_get_device_pool(struct cmd_req* data){
	PR("指令: handle_get_device_pool");
	struct cmd_req_get_device_pool* _data = (struct cmd_req_get_device_pool*)data;
	struct cmd_rep_get_device_pool* cmd_rep = (struct cmd_rep_get_device_pool*)malloc(sizeof(struct cmd_rep_get_device_pool));
	cmd_rep->size = sizeof(*cmd_rep);
	cmd_rep->ret = hsa_amd_agent_iterate_memory_pools(_data->gpu_agent, [](hsa_amd_memory_pool_t pool, void* data) {
        hsa_amd_segment_t segment;
        hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_SEGMENT, &segment);
        if (segment == HSA_AMD_SEGMENT_GLOBAL) {
            *((hsa_amd_memory_pool_t*)data) = pool;
            return HSA_STATUS_INFO_BREAK;
        }
        return HSA_STATUS_SUCCESS;
    }, &cmd_rep->device_pool);
	if(cmd_rep->ret == HSA_STATUS_INFO_BREAK)cmd_rep->ret = HSA_STATUS_SUCCESS;

    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_signal_create(struct cmd_req* data){
	PR("指令: handle_signal_create");
	struct cmd_req_signal_create* _data = (struct cmd_req_signal_create*)data;
	struct cmd_rep_signal_create* cmd_rep = (struct cmd_rep_signal_create*)malloc(sizeof(struct cmd_rep_signal_create));
	cmd_rep->size = sizeof(*cmd_rep);
	cmd_rep->ret = hsa_signal_create(_data->initial_value, 0, NULL, &cmd_rep->signal);
	PR("分配 signal:%p",cmd_rep->signal);
    return (struct hanlder_rep*)cmd_rep;
}
static struct hanlder_rep* handle_queue_create(struct cmd_req* data){
	PR("指令: handle_queue_create");
	struct cmd_req_queue_create* _data = (struct cmd_req_queue_create*)data;
	struct cmd_rep_queue_create* cmd_rep = (struct cmd_rep_queue_create*)malloc(sizeof(struct cmd_rep_queue_create));
	cmd_rep->size = sizeof(*cmd_rep);
	PR("参数 agent:%p",_data->agent.handle);
	cmd_rep->ret = hsa_queue_create(_data->agent, _data->size, _data->type, NULL, NULL, 0, 0, &cmd_rep->queue);
	PR("分配 queue:%p size:0x%llx",cmd_rep->queue,cmd_rep->queue->size);
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_load_module(struct cmd_req* data){
	PR("指令: handle_load_module");
	struct cmd_req_load_module* _data = (struct cmd_req_load_module*)data;
	struct cmd_rep_load_module* cmd_rep = (struct cmd_rep_load_module*)malloc(sizeof(struct cmd_rep_load_module));
	cmd_rep->size = sizeof(*cmd_rep);

	hsa_code_object_reader_t code_reader;

	char* kernel_code = (char*)malloc(32*1024);
	FILE* fp=fopen(_data->module_path,"r");
	if(!fp){
		cmd_rep->ret = HSA_STATUS_ERROR_INVALID_ARGUMENT;
		return (struct hanlder_rep*)cmd_rep;
	}
	fseek(fp,4096,SEEK_CUR);
	long kernel_code_len = fread(kernel_code,1,32*1024,fp);
	fclose(fp);

	cmd_rep->ret = hsa_executable_create_alt(HSA_PROFILE_FULL, HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT, nullptr, &cmd_rep->executable);
	if(cmd_rep->ret)return (struct hanlder_rep*)cmd_rep;
	PR("分配 executable:%p",cmd_rep->executable);

	cmd_rep->ret = hsa_code_object_reader_create_from_memory(kernel_code, kernel_code_len, &code_reader);
	if(cmd_rep->ret)return (struct hanlder_rep*)cmd_rep;

	cmd_rep->ret = hsa_executable_load_agent_code_object(cmd_rep->executable, _data->agent, code_reader, nullptr, nullptr);
	if(cmd_rep->ret)return (struct hanlder_rep*)cmd_rep;

	cmd_rep->ret = hsa_executable_freeze(cmd_rep->executable, "");
	if(cmd_rep->ret)return (struct hanlder_rep*)cmd_rep;

	free(kernel_code);
    return (struct hanlder_rep*)cmd_rep;
}
static struct hanlder_rep* handle_load_kernel(struct cmd_req* data){
	PR("指令: handle_load_kernel");
	struct cmd_req_load_kernel* _data = (struct cmd_req_load_kernel*)data;
	struct cmd_rep_load_kernel* cmd_rep = (struct cmd_rep_load_kernel*)malloc(sizeof(struct cmd_rep_load_kernel));
	cmd_rep->size = sizeof(*cmd_rep);

	// 解析参数与返回值
	hsa_executable_symbol_t kernel_symbol;
	cmd_rep->ret = hsa_executable_get_symbol(_data->executable, NULL, _data->kernel_name, _data->agent, 0, &kernel_symbol);
	if(cmd_rep->ret){
		PRE("handle_load_kernel failed 1");
		return (struct hanlder_rep*)cmd_rep;
	}

	cmd_rep->ret = hsa_executable_symbol_get_info(kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &cmd_rep->kernel_object);
	if(cmd_rep->ret){
		PRE("handle_load_kernel failed 2");
		return (struct hanlder_rep*)cmd_rep;
	}

    cmd_rep->ret = hsa_executable_symbol_get_info(kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &cmd_rep->kernarg_segment_size);
	if(cmd_rep->ret){
		PRE("handle_load_kernel failed 3");
		return (struct hanlder_rep*)cmd_rep;
	}
	
	cmd_rep->ret = hsa_executable_symbol_get_info(kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &cmd_rep->group_segment_size);
	if(cmd_rep->ret){
		PRE("handle_load_kernel failed 4");
		return (struct hanlder_rep*)cmd_rep;
	}
	
	cmd_rep->ret = hsa_executable_symbol_get_info(kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &cmd_rep->private_segment_size);
	if(cmd_rep->ret){
		PRE("handle_load_kernel failed 5");
		return (struct hanlder_rep*)cmd_rep;
	}

	PR("分配 kernel_addr:%p arg_size:0x%llx group_size:0x%llx private_size:0x%llx",cmd_rep->kernel_object,cmd_rep->kernarg_segment_size,cmd_rep->group_segment_size,cmd_rep->private_segment_size);
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_launch_kernel(struct cmd_req* data, hsa_signal_t completion_signal){
	PR("指令: handle_launch_kernel");
	struct cmd_req_launch_kernel* _data = (struct cmd_req_launch_kernel*)data;
	struct cmd_rep_launch_kernel* cmd_rep = (struct cmd_rep_launch_kernel*)malloc(sizeof(struct cmd_rep_launch_kernel));
	cmd_rep->size = sizeof(*cmd_rep);

	const uint32_t queue_mask = _data->queue->size - 1;
	const uint64_t index = hsa_queue_add_write_index_relaxed(_data->queue, 1);
	PR("指令序号:%d",index);
	hsa_kernel_dispatch_packet_t* packet = &((hsa_kernel_dispatch_packet_t*)(_data->queue->base_address))[index & queue_mask];
	memset(packet, 0, sizeof(*packet));
	packet->workgroup_size_x = _data->workgroup_size_x;
	packet->workgroup_size_y = _data->workgroup_size_y;
	packet->workgroup_size_z = _data->workgroup_size_z;
	packet->grid_size_x = _data->grid_size_x;
	packet->grid_size_y = _data->grid_size_y;
	packet->grid_size_z = _data->grid_size_z;
	packet->kernel_object = _data->kernel_object;
	packet->completion_signal = completion_signal;
	packet->kernarg_address = _data->args;
	PR("核函数参数:");
	PR("workgroup_size: %d %d %d",packet->workgroup_size_x,packet->workgroup_size_y,packet->workgroup_size_z);
	PR("grid_sizez: %d %d %d",packet->grid_size_x,packet->grid_size_y,packet->grid_size_z);
	PR("kernel_object: %p",packet->kernel_object);
	PR("completion_signal: %p",packet->completion_signal);
	PR("kernarg_address: %p",packet->kernarg_address);

	packet->header = _data->header;
	packet->setup = _data->setup;
	hsa_signal_store_relaxed(_data->queue->doorbell_signal, index);

	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_launch_kernel_with_signal(struct cmd_req* data){
	PR("指令: handle_launch_kernel");
	struct cmd_req_launch_kernel* _data = (struct cmd_req_launch_kernel*)data;
	return handle_launch_kernel(data, _data->completion_signal);
}

static struct hanlder_rep* handle_alloc(struct cmd_req* data){
	PR("指令: handle_alloc");
	struct cmd_req_alloc* _data = (struct cmd_req_alloc*)data;
	struct cmd_rep_alloc* cmd_rep = (struct cmd_rep_alloc*)malloc(sizeof(struct cmd_rep_alloc));
	cmd_rep->size = sizeof(*cmd_rep);

	if(_data->size<4096){
        PRE("大小错误");
    }
	if(_data->size%4096){
        PRE("大小未对齐");
    }
	cmd_rep->addr= aligned_alloc(4096,_data->size);
	PR("分配用户态空间%p size:0x%llx",cmd_rep->addr, _data->size);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_free(struct cmd_req* data){
	PR("指令: handle_free");
	struct cmd_req_free* _data = (struct cmd_req_free*)data;
	struct cmd_rep_free* cmd_rep = (struct cmd_rep_free*)malloc(sizeof(struct cmd_rep_free));
	cmd_rep->size = sizeof(*cmd_rep);
	free(_data->addr);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

//
static struct hanlder_rep* handle_pool_alloc(struct cmd_req* data){
	PR("指令: handle_pool_alloc");
	struct cmd_req_pool_alloc* _data = (struct cmd_req_pool_alloc*)data;
	struct cmd_rep_pool_alloc* cmd_rep = (struct cmd_rep_pool_alloc*)malloc(sizeof(struct cmd_rep_pool_alloc));
	cmd_rep->size = sizeof(*cmd_rep);

	if(_data->size<4096){
        PRE("大小错误 %d<4096", _data->size);
    }
	if(_data->size%4096){
        PRE("大小未对齐");
    }
	hsa_amd_memory_pool_allocate(_data->pool, _data->size, 0, &cmd_rep->addr);
	PR("分配pool空间%p size:0x%llx", cmd_rep->addr, _data->size);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_pool_free(struct cmd_req* data){
	PR("指令: handle_pool_free");
	struct cmd_req_pool_free* _data = (struct cmd_req_pool_free*)data;
	struct cmd_rep_pool_free* cmd_rep = (struct cmd_rep_pool_free*)malloc(sizeof(struct cmd_rep_pool_free));
	cmd_rep->size = sizeof(*cmd_rep);
	hsa_amd_memory_pool_free(_data->addr);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_region_alloc(struct cmd_req* data){
	PR("指令: handle_region_alloc");
	struct cmd_req_region_alloc* _data = (struct cmd_req_region_alloc*)data;
	struct cmd_rep_region_alloc* cmd_rep = (struct cmd_rep_region_alloc*)malloc(sizeof(struct cmd_rep_region_alloc));
	cmd_rep->size = sizeof(*cmd_rep);

	if(_data->size<4096){
        PRE("大小错误");
    }
	if(_data->size%4096){
        PRE("大小未对齐");
    }
    hsa_memory_allocate(_data->region, _data->size, &cmd_rep->addr);
	PR("分配region空间%p size:0x%llx",cmd_rep->addr, _data->size);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_region_free(struct cmd_req* data){
	PR("指令: handle_region_free");
	struct cmd_req_region_free* _data = (struct cmd_req_region_free*)data;
	struct cmd_rep_region_free* cmd_rep = (struct cmd_rep_region_free*)malloc(sizeof(struct cmd_rep_region_free));
	cmd_rep->size = sizeof(*cmd_rep);
	hsa_memory_free(_data->addr);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

static struct hanlder_rep* handle_copy(struct cmd_req* data){
	PR("指令: handle_copy");
	struct cmd_req_copy* _data = (struct cmd_req_copy*)data;
	struct cmd_rep_copy* cmd_rep = (struct cmd_rep_copy*)malloc(sizeof(struct cmd_rep_copy));
	cmd_rep->size = sizeof(*cmd_rep);
	hsa_amd_memory_async_copy(_data->dst,_data->gpu_agent,_data->src,_data->gpu_agent,_data->size,0,NULL,_data->completion_signal);
	cmd_rep->ret = HSA_STATUS_SUCCESS;
    return (struct hanlder_rep*)cmd_rep;
}

//*
static struct hanlder_rep* handle_queue_add_write_index_relaxed(struct cmd_req* data){
	PR("指令: handle_queue_add_write_index_relaxed");
	struct cmd_req_queue_add_write_index_relaxed* _data = (struct cmd_req_queue_add_write_index_relaxed*)data;
	struct cmd_rep_queue_add_write_index_relaxed* cmd_rep = (struct cmd_rep_queue_add_write_index_relaxed*)malloc(sizeof(struct cmd_rep_queue_add_write_index_relaxed));
	cmd_rep->size = sizeof(*cmd_rep);

	// 解析参数与返回值
	cmd_rep->ret = HSA_STATUS_SUCCESS;
	cmd_rep->index = hsa_queue_add_write_index_relaxed(_data->queue,_data->value);
    return (struct hanlder_rep*)cmd_rep;
}
static struct hanlder_rep* handle_signal_store_relaxed(struct cmd_req* data){
	PR("指令: handle_signal_store_relaxed");
	struct cmd_req_signal_store_relaxed* _data = (struct cmd_req_signal_store_relaxed*)data;
	struct cmd_rep_signal_store_relaxed* cmd_rep = (struct cmd_rep_signal_store_relaxed*)malloc(sizeof(struct cmd_rep_signal_store_relaxed));
	cmd_rep->size = sizeof(*cmd_rep);

	cmd_rep->ret = HSA_STATUS_SUCCESS;
	hsa_signal_store_relaxed(_data->signal, _data->value);
    return (struct hanlder_rep*)cmd_rep;
}
static struct hanlder_rep* handle_signal_wait_acquire(struct cmd_req* data){
	PR("指令: handle_signal_wait_acquire");
	struct cmd_req_signal_wait_acquire* _data = (struct cmd_req_signal_wait_acquire*)data;
	struct cmd_rep_signal_wait_acquire* cmd_rep = (struct cmd_rep_signal_wait_acquire*)malloc(sizeof(struct cmd_rep_signal_wait_acquire));
	cmd_rep->size = sizeof(*cmd_rep);

	cmd_rep->ret = HSA_STATUS_SUCCESS;
	PR("args:%llx %llx %llx %llx %llx",_data->signal, _data->condition, _data->compare_value, _data->timeout_hint, _data->wait_state_hint);
	cmd_rep->complete = hsa_signal_wait_acquire(_data->signal, _data->condition, _data->compare_value, _data->timeout_hint, _data->wait_state_hint);
	PR("wait end");
    return (struct hanlder_rep*)cmd_rep;
}
//*/


struct hanlder_rep* handle_cmd(struct cmd_req* data){
	PR("指令: %d",data->api_id);
    switch (data->api_id){
    case MAXAPI_empty:
		return handle_empty(data);
    case MAXAPI_init:
        return handle_init(data);
	case MAXAPI_get_gpu_agent:
        return handle_get_gpu_agent(data);
	case MAXAPI_get_kernarg_region:
        return handle_get_kernarg_region(data);
	case MAXAPI_get_device_pool:
        return handle_get_device_pool(data);
	case MAXAPI_signal_create:
		return handle_signal_create(data);
	case MAXAPI_queue_create:
		return handle_queue_create(data);
	case MAXAPI_load_module:
		return handle_load_module(data);
	case MAXAPI_load_kernel:
		return handle_load_kernel(data);
	case MAXAPI_launch_kernel:
		return handle_launch_kernel(data, {0});
	case MAXAPI_launch_kernel_with_signal:
		return handle_launch_kernel_with_signal(data);
	case MAXAPI_alloc:
		return handle_alloc(data);
	case MAXAPI_free:
		return handle_free(data);
	case MAXAPI_pool_alloc:
		return handle_pool_alloc(data);
	case MAXAPI_pool_free:
		return handle_pool_free(data);
	case MAXAPI_region_alloc:
		return handle_region_alloc(data);
	case MAXAPI_region_free:
		return handle_region_free(data);
	case MAXAPI_copy:
		return handle_copy(data);
	case MAXAPI_queue_add_write_index_relaxed:
		return handle_queue_add_write_index_relaxed(data);
	case MAXAPI_signal_store_relaxed:
		return handle_signal_store_relaxed(data);
	case MAXAPI_signal_wait_acquire:
		return handle_signal_wait_acquire(data);
	default:
        PRE("未知指令:%d", data->api_id);
        return NULL;
    }
	return NULL;
}