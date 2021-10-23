#include <unistd.h>
#include <libc/syscall.h>

int setpgid(pid_t pid, pid_t pgid)
{
    return SYSCALL_ERR(setpgid, pid, pgid);
}