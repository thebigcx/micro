#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fs.h>

SYSCALL_DEFINE(getdents, int fdno, struct dirent* dirp, size_t n)
{
    PTRVALID(dirp);
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    if (fd->filp->flags != FL_DIR) return -ENOTDIR;

    return vfs_getdents(fd->filp, fd->off++, n, dirp);
}