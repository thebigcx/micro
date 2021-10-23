#include <unistd.h>
#include <libc/syscall.h>

pid_t getpgid(pid_t pid)
{
    return SYSCALL_ERR(getpgid, pid);
}