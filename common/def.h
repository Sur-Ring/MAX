#ifndef __MAX_DEF_H__
#define __MAX_DEF_H__

// 调试输出
#ifdef __MAX_KERNEL__
    typedef signed char __int8_t;
    typedef unsigned char __uint8_t;
    typedef signed short int __int16_t;
    typedef unsigned short int __uint16_t;
    typedef signed int __int32_t;
    typedef unsigned int __uint32_t;
    __extension__ typedef signed long long int __int64_t;
    __extension__ typedef unsigned long long int __uint64_t;

    typedef __uint8_t uint8_t;
    typedef __uint16_t uint16_t;
    typedef __uint32_t uint32_t;
    typedef __uint64_t uint64_t;

    typedef __int8_t int8_t;
    typedef __int16_t int16_t;
    typedef __int32_t int32_t;
    typedef __int64_t int64_t;

    typedef __SIZE_TYPE__ size_t;

    #define PR(fmt,...) ;
    // #define PR(fmt,...) printk(KERN_INFO fmt , ##__VA_ARGS__)
    #define PRI(fmt,...) printk(KERN_INFO "BY: %s: " fmt "\n", __func__, ##__VA_ARGS__)
    #define PRE(fmt,...) printk(KERN_ERR "BY: %s-%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

#else
    #define PR(fmt,...) ;
    // #define PR(fmt,...) printf("BY: %s: " fmt "\n", __func__, ##__VA_ARGS__);fflush(0)
    #define PRI(fmt,...) printf("BY: %s: " fmt "\n", __func__, ##__VA_ARGS__);fflush(0)
    #define PRE(fmt,...) printf("\033[1;31;40mBY_ERROR\033[0m: %s-%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);fflush(0)
#endif

// netlink
#define NETLINK_USER 30

#define NETLINK_REQ      0x11
#define NETLINK_REP      0x12

#define PAGE_SIZE_ALIGN(ptr) ((ptr)&~(PAGE_SIZE-1))
#define PAGE_SIZE_OFFSET(ptr) ((ptr)&(PAGE_SIZE-1))

#endif