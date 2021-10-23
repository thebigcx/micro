#include <sys/time.h>
#include <libc/syscall.h>

int utimes(const char* path, const struct timeval times[2])
{
    return SYSCALL_ERR(utimes, path, times);
}