#include <unistd.h>
#include <libc/syscall.h>

int mkdir(const char* path, mode_t mode)
{
    return SYSCALL_ERR(mkdir, path, mode);
}