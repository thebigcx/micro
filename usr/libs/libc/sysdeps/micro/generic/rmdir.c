#include <unistd.h>
#include <libc/syscall.h>

int rmdir(const char* path)
{
    return SYSCALL_ERR(rmdir, path);
}