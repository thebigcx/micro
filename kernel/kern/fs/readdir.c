#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fs.h>

SYSCALL_DEFINE(getdents, int fdno, struct dirent* dirp, size_t n)
{
    PTRVALID(dirp);
    FDVALID(fdno);

    struct file* fd = task_curr()->fds[fdno];

    if (!S_ISDIR(fd->inode->mode)) return -ENOTDIR;

    return vfs_getdents(fd->inode, fd->off++, n, dirp);
}