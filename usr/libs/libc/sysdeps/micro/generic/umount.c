#include <unistd.h>
#include <libc/syscall.h>

int umount(const char* target)
{
    return SYSCALL_ERR(umount, target);
}