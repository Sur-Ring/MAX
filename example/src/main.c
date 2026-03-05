#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/io.h> 

#include "../../common/def.h"
#include "../../common/hsa_def.h"
#include "../../common/api_kernel.h"

#define SIZE 64
# define UINT64_MAX		(__UINT64_C(18446744073709551615))

// 内核参数结构体
typedef struct {
    float* a;
    float* b;
} __attribute__((aligned(16))) kernel_args_t;

uint64_t tmp_wait(MAX_signal signal){
    return ioread32(signal.doorbell_ptr)!=0;
}

int my_wait(MAX_signal signal){
    struct timespec64 start,cur;
    ktime_get_real_ts64(&start); // 获取真实时间（秒 + 纳秒）
    while (ioread32(signal.doorbell_ptr)!=0){
        ktime_get_real_ts64(&cur); // 获取真实时间（秒 + 纳秒）
        if((cur.tv_sec-start.tv_sec)*1000000000+(cur.tv_nsec-start.tv_nsec) > 1000000000){
            PRE("error doorbell:%d",ioread32(signal.doorbell_ptr));
            return -1;
        }
    }
    return 0;
}

hsa_region_t kernarg_region;
hsa_amd_memory_pool_t device_pool;

typedef struct {
    void* a;
    void* b;
}kernel_args;

kernel_info vec_add_kernel;

address_pair A;
address_pair B;
address_pair d_A;
address_pair d_B;

address_pair d_args;

void my_test(void){
    hsa_agent_t gpu_agent;
    MAX_queue queue;
    MAX_signal signal;
    
    // 1. 初始化HSA运行时
    MAX_init();
    MAX_get_gpu_agent(&gpu_agent);
    MAX_get_kernarg_region(gpu_agent, &kernarg_region);
    MAX_get_device_pool(gpu_agent, &device_pool);
    MAX_signal_create(1, &signal);
    MAX_queue_create(gpu_agent, 256, HSA_QUEUE_TYPE_MULTI, &queue);

    // 6. 读取核函数代码
    hsa_executable_t executable;
    MAX_load_module(gpu_agent,"/home/jby/MAX/example/src/module.co",&executable);
	printk(KERN_INFO "获得 executable:0x%llx\n",executable);
    MAX_load_kernel(gpu_agent,executable,"vec_add.kd",&vec_add_kernel);

    // 8. 准备核函数参数
    MAX_alloc(4*sizeof(int),&A);
    MAX_alloc(4*sizeof(int),&B);
    MAX_pool_alloc(device_pool,4*sizeof(int),&d_A);
    MAX_pool_alloc(device_pool,4*sizeof(int),&d_B);
    MAX_region_alloc(kernarg_region, 512, &d_args);

    kernel_args h_args = {
        d_A.user, d_B.user
    };
    memcpy(d_args.kernel,&h_args,sizeof(h_args));

    for(int i=0;i<4;i++){
        ((int*)A.kernel)[i]=i;
        ((int*)B.kernel)[i]=2*i;
    }
    MAX_signal copy_signal;
    MAX_signal_create(1, &copy_signal);

    for(int i=0;i<4;i++){
        PRI("input[%d]:%d %d",i,((int*)A.kernel)[i], ((int*)B.kernel)[i]);
    }

    MAX_copy(gpu_agent,d_A.user,A.user,4*sizeof(int),copy_signal);
    my_wait(copy_signal);
    iowrite32(1, copy_signal.doorbell_ptr);

    MAX_copy(gpu_agent,d_B.user,B.user,4*sizeof(int),copy_signal);
    my_wait(copy_signal);
    iowrite32(1, copy_signal.doorbell_ptr);

    // // 9. 提交核函数到队列
    uint16_t header = (HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE) |
                      (HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE) |
                      (HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE);
    uint16_t setup = 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;

    const uint32_t queue_mask = queue.kern_queue->size - 1;
    uint64_t index = (*queue.wptr)++;
    hsa_kernel_dispatch_packet_t* packet = &((hsa_kernel_dispatch_packet_t*)(queue.kern_ringbuffer))[index & queue_mask];
    memset(packet, 0, sizeof(*packet));
    packet->workgroup_size_x = 4;
    packet->workgroup_size_y = 1;
    packet->workgroup_size_z = 1;
    packet->grid_size_x = 4;
    packet->grid_size_y = 1;
    packet->grid_size_z = 1;
    packet->completion_signal = signal.user_signal;
    packet->kernel_object = vec_add_kernel.kernel_object;
    packet->kernarg_address = d_args.user;
    packet->header = header;
    packet->setup = setup;
    iowrite32(index, queue.doorbell_signal.doorbell_ptr);
    if(my_wait(signal)==-1){
        PRI("等待错误\n");
        return;
    }
    iowrite32(1, signal.doorbell_ptr);

    MAX_copy(gpu_agent,B.user,d_B.user,4*sizeof(int),copy_signal);
    my_wait(copy_signal);
    iowrite32(1, copy_signal.doorbell_ptr);

    for(int i=0;i<4;i++){
        PRI("output[%d]:%d",i,((int*)B.kernel)[i]);
    }
}

static int __init my_module_init(void){
    printk(KERN_INFO "my_module_init:%s\n",MODULE_NAME);

    my_test();
    
    return 0;
}

static void __exit my_module_exit(void){
    printk(KERN_INFO "wait my_module_exit\n");
    synchronize_rcu();
    printk(KERN_INFO "my_module_exit\n");
}

module_init(my_module_init); module_exit(my_module_exit);

MODULE_LICENSE("GPL");