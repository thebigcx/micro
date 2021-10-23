#include <fcntl.h>
#include <stdarg.h>
#include <libc/syscall.h>

int open(const char* filename, int flags, ...)
{
    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    va_end(args);

    return SYSCALL_ERR(open, filename, flags, mode);
}