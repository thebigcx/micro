#include <sys/ptrace.h>
#include <libc/syscall.h>

long ptrace(enum __ptrace_request req, pid_t pid, void* addr, void* data)
{
    return SYSCALL_ERR(ptrace, req, pid, addr, data);
}