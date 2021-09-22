#include <sys/time.h>
#include <libc/syscall.h>

int gettimeofday(struct timeval* tv,
                 void* tz)
{
    return SYSCALL_ERR(gettimeofday, tv, tz);
}