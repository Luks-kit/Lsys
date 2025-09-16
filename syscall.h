#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

static inline long sys_write(int fd, const void *buf, unsigned long len) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(1), "D"(fd), "S"(buf), "d"(len)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline void sys_exit(int code) {
    asm volatile(
        "syscall"
        :
        : "a"(60), "D"(code)
        : "rcx", "r11", "memory"
    );
    __builtin_unreachable();
}

#endif

