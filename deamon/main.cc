#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "./include/netlink.h"

volatile sig_atomic_t stop_running = 0;

void exit_handler(int dummy) {
    stop_running = 1;
    //TODO: sock is blocking, so it never quits the loop, just quit here
    sleep(1);
    exit_nl();
    exit(0);
}

int main() {
    // 注册退出函数
    signal(SIGINT, exit_handler);
    
    // 与内核模块建立链接
    PRI("Starting uspace max daemon with pid %d", getpid());fflush(0);
    init_nl();

    while(!stop_running) {
        recv_nl();
    }
    
    PRI("Quitting");fflush(0);
    return 0;
}

