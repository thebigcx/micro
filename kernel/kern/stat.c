#include <micro/sys.h>
#include <micro/stat.h>
#include <micro/vfs.h>
#include <micro/heap.h>

static void do_kstat(struct file* file, struct stat* buf)
{
    buf->st_dev     = 0;
    buf->st_ino     = file->inode;
    buf->st_mode    = file->type | file->perms;
    buf->st_nlink   = file->links;
    buf->st_uid     = file->uid;
    buf->st_gid     = file->gid;
    buf->st_rdev    = (file->major << 32) | file->minor;
    buf->st_size    = file->size;
    buf->st_blksize = 1024;
    buf->st_blocks  = 0;
    buf->st_atime   = file->atime;
    buf->st_mtime   = file->mtime;
    buf->st_ctime   = file->ctime;
}

SYSCALL_DEFINE(stat, const char* path, struct stat* buf)
{
    PTRVALID(path);
    PTRVALID(buf);

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    struct file file;
    int e = vfs_resolve(canon, &file);

    kfree(canon);
    if (e) return e;

    do_kstat(&file, buf);
    return 0;
}

SYSCALL_DEFINE(fstat, int fd, struct stat* buf)
{
    PTRVALID(buf);
    FDVALID(fd);

    do_kstat(task_curr()->fds[fd]->filp, buf);
    return 0;
}

SYSCALL_DEFINE(lstat, const char* path, struct stat* buf)
{
    // TODO: symbolic links
    PTRVALID(path);
    PTRVALID(buf);

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    struct file file;
    int e = vfs_resolve(canon, &file);

    kfree(canon);
    if (e) return e;

    do_kstat(&file, buf);
    return 0;
}