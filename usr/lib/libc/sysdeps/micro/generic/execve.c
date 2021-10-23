#include <unistd.h>
#include <libc/syscall.h>

int execve(const char* pathname, const char* argv[], const char* envp[])
{
    return SYSCALL_ERR(execve, pathname, argv, envp);
}