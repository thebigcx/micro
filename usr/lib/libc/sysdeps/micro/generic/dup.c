#include <unistd.h>
#include <libc/syscall.h>

int dup(int oldfd)
{
    return SYSCALL_ERR(dup, oldfd);
}