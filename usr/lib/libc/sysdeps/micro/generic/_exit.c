#include <unistd.h>
#include <sys/syscall.h>

void _exit(int status)
{
    syscall(SYS_exit, status);
}