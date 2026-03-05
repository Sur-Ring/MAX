#include <linux/module.h>
#include <linux/kernel.h>

#include "netlink.h"
#include "shm.h"

static int __init my_module_init(void){
    PRI("my_module_init:%s",MODULE_NAME);
    shm_init();
    netlink_init();

    return 0;
}

static void __exit my_module_exit(void){
    PRI("my_module_exit");
    netlink_exit();
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
