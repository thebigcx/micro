#include <micro/sys.h>
#include <micro/vfs.h>

SYSCALL_DEFINE(ioctl, int fdno, int req, void* argp)
{
    printk("ioctl(%d, %x)\n", fdno, req);
    FDVALID(fdno);
    PTRVALIDNULL(argp);
    
    struct file* fd = task_curr()->fds[fdno];

    if (S_ISREG(fd->inode->mode) || S_ISDIR(fd->inode->mode)) return -ENOTTY;

    return vfs_ioctl(fd, req, argp);
}