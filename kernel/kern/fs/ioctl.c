#include <micro/sys.h>
#include <micro/vfs.h>

SYSCALL_DEFINE(ioctl, int fdno, unsigned long req, void* argp)
{
    FDVALID(fdno);
    PTRVALID(argp);
    
    struct file* fd = task_curr()->fds[fdno];

    if (S_ISREG(fd->inode->mode) || S_ISDIR(fd->inode->mode)) return -ENOTTY;

    return vfs_ioctl(fd, req, argp);
}