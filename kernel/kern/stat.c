#include <micro/sys.h>
#include <micro/stat.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/heap.h>
#include <micro/try.h>

static void do_kstat(struct inode* inode, struct stat* buf)
{
    buf->st_dev     = 0;
    buf->st_ino     = inode->inode;
    buf->st_mode    = inode->mode;
    buf->st_nlink   = inode->nlink;
    buf->st_uid     = inode->uid;
    buf->st_gid     = inode->gid;
    buf->st_rdev    = inode->rdev;
    buf->st_size    = inode->size;
    buf->st_blksize = inode->blksize;
    buf->st_blocks  = inode->blocks;
    
    buf->st_atim.tv_sec = inode->atime;
    buf->st_mtim.tv_sec = inode->mtime;
    buf->st_ctim.tv_sec = inode->ctime;
}

SYSCALL_DEFINE(stat, const char* path, struct stat* buf)
{
    PTRVALID(path);
    PTRVALID(buf);

    char canon[256];
    vfs_mkcanon(path, task_curr()->workd, canon);

    struct file file;
    TRY(vfs_open(canon, &file, O_RDONLY));

    do_kstat(file.inode, buf);
    return 0;
}

SYSCALL_DEFINE(fstat, int fd, struct stat* buf)
{
    PTRVALID(buf);
    FDVALID(fd);

    do_kstat(task_curr()->fds[fd]->inode, buf);
    return 0;
}

SYSCALL_DEFINE(lstat, const char* path, struct stat* buf)
{
    PTRVALID(path);
    PTRVALID(buf);

    char canon[256];
    vfs_mkcanon(path, task_curr()->workd, canon);

    struct file file;
    TRY(vfs_open(canon, &file, O_RDONLY | O_NOFOLLOW | O_PATH));

    do_kstat(file.inode, buf);
    printk("mode: %d\n", (uintptr_t)&buf->st_mode - (uintptr_t)buf);
    printk("%d\n", buf->st_blocks);
    return 0;
}

SYSCALL_DEFINE(readlink, const char* pathname, char* buf, size_t n)
{
    PTRVALID(pathname);
    PTRVALID(buf);

    char canon[256];
    vfs_mkcanon(pathname, task_curr()->workd, canon);

    struct file file;
    TRY(vfs_open(canon, &file, O_RDONLY | O_NOFOLLOW | O_PATH));

    if (!S_ISLNK(file.inode->mode)) return -EINVAL;

    return vfs_readlink(file.inode, buf, n);
}
