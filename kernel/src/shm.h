#ifndef __MAX_SHM_H__
#define __MAX_SHM_H__

struct map_node{
    void* user_addr;
    void* kern_addr;
    int page_count;
    struct page** pages;
    // struct page ** pages;
    struct map_node* next;
};

void shm_init(void);
void shm_change_deamon(u32 deamon_pid);
void remap_queue(hsa_queue_t * user_queue, MAX_queue* queue);
void remap_signal(hsa_signal_t user_signal, MAX_signal* signal);
void* remap_alloc(void* user_virt, size_t size);
struct map_node* remap_node(void* user_base, size_t size);
void* remap_addr(void* user_virt, size_t size);
void unremap_node(struct map_node*);
void unremap_addr(void* kern_virt);
void* get_user_virt(void* kern_virt);

#endif