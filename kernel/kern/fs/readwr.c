#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>

SYSCALL_DEFINE(read, int fdno, void* buf, size_t size)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    
    struct file* fd = task->fds[fdno];
    return vfs_read(fd, buf, size);
}

SYSCALL_DEFINE(write, int fdno, const void* buf, size_t size)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    
    struct file* fd = task->fds[fdno];

    if (fd->flags & O_APPEND) fd->off = fd->inode->size;

    return vfs_write(fd, buf, size);
}

SYSCALL_DEFINE(pread, int fdno, void* buf, size_t size, off_t off)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    return vfs_pread(task->fds[fdno], buf, size, off);
}

SYSCALL_DEFINE(pwrite, int fdno, const void* buf, size_t size, off_t off)
{
    PTRVALID(buf);
    FDVALID(fdno);

    struct task* task = task_curr();
    return vfs_pwrite(task->fds[fdno], buf, size, off);
}

SYSCALL_DEFINE(lseek, int fdno, off_t offset, int whence)
{
    FDVALID(fdno);

    struct file* fd = task_curr()->fds[fdno];

    switch (whence)
    {
        case SEEK_SET:
            if (fd->flags & O_APPEND)
                fd->off = offset + fd->inode->size;
            else
                fd->off = offset;

            return fd->off;

        case SEEK_END:
            fd->off = fd->inode->size + offset;
            return fd->off;
        
        case SEEK_CUR:
            fd->off += offset;
            return fd->off;
    }

    return -EINVAL;
}

SYSCALL_DEFINE(readv, int fd, const struct iovec* iov, int iovcnt)
{
    FDVALID(fd);
    PTRVALID(iov);

    if (iovcnt < 0) return -EINVAL;

    struct file* file = task_curr()->fds[fd];
    
    ssize_t bytes = 0;
    for (int i = 0; i < iovcnt; i++)
    {
        PTRVALID(iov + i);
        
        ssize_t ret = vfs_read(file, iov[i].iov_base, iov[i].iov_len);
        if (ret < 0)
            return ret;

        bytes += ret;
    }

    return bytes;
}

SYSCALL_DEFINE(writev, int fd, const struct iovec* iov, int iovcnt)
{
    FDVALID(fd);
    PTRVALID(iov);

    if (iovcnt < 0) return -EINVAL;

    struct file* file = task_curr()->fds[fd];
    
    ssize_t bytes = 0;
    for (int i = 0; i < iovcnt; i++)
    {
        PTRVALID(iov + i);
        PTRVALID(iov[i].iov_base);
        
        ssize_t ret = vfs_write(file, iov[i].iov_base, iov[i].iov_len);
        if (ret < 0)
            return ret;

        bytes += ret;
    }

    return bytes;
}