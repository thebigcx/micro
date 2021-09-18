#pragma once

#include <sys/syscall.h>
#include <errno.h>

// Syscall that should return -1 on on error
__attribute__((always_inline))
inline int __syscall_err(int ret)
{
    if (ret < 0)
    {
        errno = ret;
        return -1;
    }
    return ret;
}

// Syscall that should return NULL on error
__attribute__((always_inline))
inline unsigned long __syscall_err_null(long ret)
{
    if (ret < 0)
    {
        errno = ret;
        return 0;
    }
    return ret;
}

#define SYSCALL_ERR(sys, ...) (__syscall_err(syscall(SYS_##sys, ##__VA_ARGS__)))

#define SYSCALL_ERR_NULL(sys, ...) (__syscall_err_null(syscall(SYS_#sys, ##__VA_ARGS__)))