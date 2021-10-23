#include <time.h>
#include <libc/syscall.h>

int nanosleep(const struct timespec* req, struct timespec* rem)
{
    //return SYSCALL_ERR(nanosleep, req, rem);
    return 0;
}