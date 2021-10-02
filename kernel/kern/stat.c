#include <micro/sys.h>
#include <micro/stat.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/heap.h>

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
    buf->st_atime   = inode->atime;
    buf->st_mtime   = inode->mtime;
    buf->st_ctime   = inode->ctime;
}

SYSCALL_DEFINE(stat, const char* path, struct stat* buf)
{
    PTRVALID(path);
    PTRVALID(buf);

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    struct file file;
    int e = vfs_open_new(canon, &file, O_RDONLY);

    kfree(canon);
    if (e) return e;

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

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    struct file file;
    int e = vfs_open_new(canon, &file, O_RDONLY | O_NOFOLLOW | O_PATH);

    kfree(canon);
    if (e) return e;

    do_kstat(file.inode, buf);
    return 0;
}

SYSCALL_DEFINE(readlink, const char* pathname, char* buf, size_t n)
{
    PTRVALID(pathname);
    PTRVALID(buf);

    char* canon = vfs_mkcanon(pathname, task_curr()->workd);

    struct file file;
    int e = vfs_open_new(canon, &file, O_RDONLY | O_NOFOLLOW | O_PATH);
    //struct inode file;
    //int e = vfs_resolve(canon, &file, 0);

    kfree(canon);

    if (e) return e;
    if (!S_ISLNK(file.inode->mode)) return -EINVAL;

    return vfs_readlink(file.inode, buf, n);
}