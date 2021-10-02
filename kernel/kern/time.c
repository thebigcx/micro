#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/try.h>
#include <micro/fcntl.h>
#include <micro/time.h>

int do_utime(const char* path, const struct utimbuf* times)
{
    struct file file;
    TRY(vfs_open(path, &file, O_RDWR));

    if (file.inode->uid != task_curr()->euid && task_curr()->euid)
        return -EPERM;

    if (!file.inode->ops.set_atime || !file.inode->ops.set_mtime)
        return -ENOENT;

    if (!times)
    {
        printk("warning: utime() with times=NULL is not supported!\n");
        return -ENOSYS;
    }

    TRY(file.inode->ops.set_atime(file.inode, times->actime));
    TRY(file.inode->ops.set_atime(file.inode, times->modtime));
}

// TODO: implement
SYSCALL_DEFINE(utime, const char* path, const struct utimbuf* times)
{
    PTRVALID(path);
    PTRVALIDNULL(times);

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    int ret = do_utime(canon, times);

    kfree(canon);
    return ret;
}

SYSCALL_DEFINE(utimes, const char* path, const struct timeval times[2])
{
    PTRVALID(path);
    PTRVALIDNULL(times);

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    int ret;
    if (times)
    {
        struct utimbuf buf =
        {
            .actime = times[0].tv_sec,
            .modtime = times[1].tv_sec
        };
        ret = do_utime(canon, &buf);
    }
    else
        ret = do_utime(canon, NULL);

    kfree(canon);
    return ret;
}