#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>

SYSCALL_DEFINE(read, int fdno, void* buf, size_t size)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    
    struct fd* fd = task->fds[fdno];
    ssize_t ret = vfs_read(fd->filp, buf, fd->off, size);
    fd->off += size;

    return ret;
}

SYSCALL_DEFINE(write, int fdno, const void* buf, size_t size)
{
    FDVALID(fdno);
    PTRVALID(buf);

    //printk("write: ");
    //for (size_t i = 0; i < size; i++) printk("%c", ((char*)buf)[i]);
    //printk("\n");

    struct task* task = task_curr();
    
    struct fd* fd = task->fds[fdno];

    if (fd->flags & O_APPEND) fd->off = fd->filp->size;

    ssize_t ret = vfs_write(fd->filp, buf, fd->off, size);
    fd->off += size;

    return ret;
}

SYSCALL_DEFINE(pread, int fdno, void* buf, size_t size, off_t off)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    ssize_t ret = vfs_read(task->fds[fdno]->filp, buf, off, size);

    return ret;
}

SYSCALL_DEFINE(pwrite, int fdno, const void* buf, size_t size, off_t off)
{
    PTRVALID(buf);
    FDVALID(fdno);

    struct task* task = task_curr();
    ssize_t ret = vfs_write(task->fds[fdno]->filp, buf, off, size);

    return ret;
}

SYSCALL_DEFINE(lseek, int fdno, off_t offset, int whence)
{
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    switch (whence)
    {
        case SEEK_SET:
            fd->off = offset;
            return fd->off;
        case SEEK_END:
            fd->off = fd->filp->size + offset;
            return fd->off;
        case SEEK_CUR:
            fd->off += offset;
            return fd->off;
    }

    return -EINVAL;
}