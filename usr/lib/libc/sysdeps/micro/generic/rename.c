#include <stdio.h>
#include <libc/syscall.h>

int rename(const char* old, const char* new)
{
    return SYSCALL_ERR(rename, old, new);
}