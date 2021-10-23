#include <unistd.h>
#include <libc/syscall.h>

int setreuid(uid_t ruid, uid_t euid)
{
    return SYSCALL_ERR(setreuid, ruid, euid);
}