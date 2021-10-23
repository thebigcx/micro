#include <unistd.h>
#include <libc/syscall.h>

int setuid(uid_t uid)
{
    return SYSCALL_ERR(setuid, uid);
}