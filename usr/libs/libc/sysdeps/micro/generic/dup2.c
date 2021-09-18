#include <unistd.h>
#include <libc/syscall.h>

int dup2(int oldfd, int newfd)
{
    return SYSCALL_ERR(dup2, oldfd, newfd);
}