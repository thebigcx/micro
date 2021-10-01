#include <micro/sys.h>
#include <micro/vfs.h>

SYSCALL_DEFINE(ioctl, int fdno, unsigned long req, void* argp)
{
    FDVALID(fdno);
    PTRVALID(argp);
    
    struct fd* fd = task_curr()->fds[fdno];

    if (fd->filp->type == S_IFREG || fd->filp->type == S_IFDIR) return -ENOTTY;

    return vfs_ioctl(fd->filp, req, argp);
}