#include <utime.h>
#include <libc/syscall.h>

int utime(const char* path, const struct utimbuf* times)
{
    return SYSCALL_ERR(utime, path, times);
}