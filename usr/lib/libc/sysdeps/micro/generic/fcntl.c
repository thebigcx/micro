#include <fcntl.h>
#include <stdarg.h>
#include <libc/syscall.h>

int fcntl(int fd, int cmd, ... /* arg */)
{
    va_list args;
    va_start(args, cmd);
    void* arg = va_arg(args, void*);
    va_end(args);

    return SYSCALL_ERR(fcntl, fd, cmd, arg);
}