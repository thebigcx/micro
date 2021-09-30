#include <unistd.h>
#include <sys/syscall.h>

pid_t getppid()
{
    return syscall(SYS_getppid);
}