#include <sys/stat.h>
#include <libc/syscall.h>

int mknod(const char* path, mode_t mode, dev_t dev)
{
    return SYSCALL_ERR(mknod, path, mode, dev);
}