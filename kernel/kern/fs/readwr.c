#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>

SYSCALL_DEFINE(read, int fdno, void* buf, size_t size)
{
    FDVALID(fdno);
    PTRVALID(buf);

    struct task* task = task_curr();
    
    struct file* fd = task->fds[fdno];
    //ssize_t ret = vfs_read(fd->inode, buf, fd->off, size);
    //fd->off += size;
    return vfs_read(fd, buf, size);
}

SYSCALL_DEFINE(write, int fdno, const void* buf, size_t size)
{
    FDVALID(fdno);
    PTRVALID(buf);

    //printk("write: ");
    //for (size_t i = 0; i < size; i++) printk("%c", ((char*)buf)[i]);
    //printk("\n");

    struct task* task = task_curr();
    
    struct file* fd = task->fds[fdno];

    if (fd->flags & O_APPEND) fd->off = fd->inode->size;

    //ssize_t ret = vfs_write(fd->inode, buf, fd->off, size);
    //fd->off += size;
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