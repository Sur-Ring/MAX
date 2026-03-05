#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#include "../../common/def.h"
#include "../../common/api_kernel.h"
#include "shm.h"

struct map_node* head;
struct map_node* tail;
struct task_struct* task;
struct map_node* doorbell_node;

void shm_init(void){
    head = (struct map_node*)kmalloc(sizeof(struct map_node), GFP_KERNEL);
    tail = head;
    head->next = NULL;
}

void shm_change_deamon(u32 deamon_pid){
    struct pid* pid_struct = find_get_pid(deamon_pid);
    if (!pid_struct) {
        pr_err("PID %d not found\n", deamon_pid);
        return;
    }

    task = pid_task(pid_struct, PIDTYPE_PID);
    if (!task) {
        pr_err("Process %d not found\n", deamon_pid);
        put_pid(pid_struct);
        return;
    }
}

struct map_node* remap_doorbell(void* base){
    struct map_node* node = (struct map_node*)kmalloc(sizeof(struct map_node), GFP_KERNEL);
  
    phys_addr_t phys_addr = 0xFC20004000;  // TODO：自动获取物理地址
    size_t size = 8192;
    // 映射到内核虚拟地址
    void __iomem *kernel_addr = ioremap(phys_addr, size);
    if (!kernel_addr) {
        pr_err("ioremap failed for address 0x%llx\n", phys_addr);
    }
    node->user_addr = base;
    node->kern_addr = kernel_addr;
    node->page_count = 0;

    return node;
}

void remap_queue(hsa_queue_t * user_queue, MAX_queue* queue){
    queue->user_queue = user_queue;
    PR("映射queue:0x%llx",(uint64_t)queue->user_queue);
    queue->queue_node = remap_node(queue->user_queue,4096);
    queue->kern_queue = (hsa_queue_t*)queue->queue_node->kern_addr;
    PR("映射ring buffer:0x%llx",(uint64_t)queue->kern_queue->base_address);
    queue->ring_buffer_node = remap_node(queue->kern_queue->base_address,queue->kern_queue->size*0x40);
    queue->kern_ringbuffer = queue->ring_buffer_node->kern_addr;
    queue->wptr = (void*)queue->kern_queue+0x38;
    queue->rptr = (void*)queue->kern_queue+0x80;
    PR("映射signal:0x%llx",queue->kern_queue->doorbell_signal.handle);
    queue->doorbell_signal.user_signal = queue->kern_queue->doorbell_signal;
    queue->doorbell_signal.signal_node = remap_node(PAGE_SIZE_ALIGN(queue->kern_queue->doorbell_signal.handle),4096);
    queue->doorbell_signal.kern_signal.handle = queue->doorbell_signal.signal_node->kern_addr + PAGE_SIZE_OFFSET(queue->kern_queue->doorbell_signal.handle);
    uint64_t user_addr = *(uint64_t*)(queue->doorbell_signal.kern_signal.handle+0x8);
    if(!doorbell_node){
        PR("映射doorbell:0x%llx",user_addr);
        doorbell_node = remap_doorbell(PAGE_SIZE_ALIGN(user_addr));
    }
    queue->doorbell_signal.doorbell_ptr = doorbell_node->kern_addr + (user_addr - (uint64_t)doorbell_node->user_addr);
    PR("完成 0x%llx",queue->doorbell_signal.doorbell_ptr);
}

void remap_signal(hsa_signal_t user_signal, MAX_signal* signal){
    signal->user_signal = user_signal;
    signal->signal_node = remap_node((void*)PAGE_SIZE_ALIGN(signal->user_signal.handle),4096);
    signal->kern_signal.handle = signal->signal_node->kern_addr+PAGE_SIZE_OFFSET(signal->user_signal.handle);
    signal->doorbell_ptr = ((void*)signal->kern_signal.handle+0x8);
}

void* remap_alloc(void* user_virt, size_t size){
    return NULL;
}

struct map_node* remap_node(void* user_base, size_t size){
    if(size<4096){
        PRE("大小错误");
    }
    if(size%PAGE_SIZE){
        PRE("大小未对齐");
    }
    if((uint64_t)user_base%PAGE_SIZE){
        PRE("用户地址未对齐");
    }
    int page_count = size/PAGE_SIZE;

    struct map_node* node = (struct map_node*)kmalloc(sizeof(struct map_node), GFP_KERNEL);

    if(task==NULL){
        PRE("未初始化守护进程");
        return NULL;
    }

    struct page** pages = kmalloc(sizeof(struct page*)*page_count, GFP_KERNEL);\
    for(int i=0;i<page_count;i++){
        void* user_addr = user_base + PAGE_SIZE*i;
        int ret = get_user_pages_remote(task->mm, user_addr, 1, FOLL_WRITE | FOLL_GET, &pages[i], NULL);
        if (ret != 1) {
            PRE("Failed to get page (ret=%d)", ret);
            return NULL;
        }
    }
    void* vaddr = vmap(pages, page_count, VM_MAP, PAGE_KERNEL);
    if (!vaddr) {
        PRE("vmap failed");
    }

    node->user_addr = user_base;
    node->kern_addr = vaddr;
    node->page_count = page_count;
    node->pages = pages;
	PR("最终映射用户态虚拟地址:0x%llx到内核态虚拟地址:0x%llx size:0x%llx",(uint64_t)user_base, (uint64_t)node->kern_addr, (uint64_t)page_count*4096);

    return node;
}

void* remap_addr(void* user_base, size_t size){
    struct map_node* node = remap_node(user_base, size);
    if(node==NULL)return NULL;

    tail->next = node;
    tail = node;

    return node->kern_addr;
}

void unremap_node(struct map_node* node){
    vunmap(node->kern_addr);
    kfree(node->pages);
    kfree(node);
}

void unremap_addr(void* kern_virt){
    struct map_node* prev = head;
    struct map_node* cur = head->next;
    while(cur != NULL){
        if(cur->kern_addr==kern_virt){
            break;
        }
        prev = cur;
        cur = cur->next;
    }
    if(cur == NULL){
        PRE("未找到0x%llx对应用户地址",(uint64_t)kern_virt);
        return;
    }
    prev->next = cur->next;
    if(tail==cur)tail=prev;
    unremap_node(cur);
}

void* get_user_virt(void* kern_virt){
    struct map_node* cur = head->next;
    while(cur != NULL){
        if(cur->kern_addr==kern_virt){
            return cur->user_addr;
        }
        cur = cur->next;
    }
    PRE("未找到0x%llx对应用户地址",(uint64_t)kern_virt);
    return NULL;
}