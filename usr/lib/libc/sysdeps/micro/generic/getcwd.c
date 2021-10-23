#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <libc/syscall.h>

char* getcwd(char* buf, size_t size)
{
    if (!buf)
    {
        if (!size) size = PATH_MAX;
        buf = malloc(size);
    }

    return (char*)(uintptr_t)SYSCALL_ERR(getcwd, buf, size);
}