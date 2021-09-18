#include <sys/wait.h>
#include <libc/syscall.h>

pid_t waitpid(pid_t pid, int* wstatus, int options)
{
    return SYSCALL_ERR(waitpid, pid, wstatus, options);
}