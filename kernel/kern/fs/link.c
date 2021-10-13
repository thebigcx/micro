#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/heap.h>

SYSCALL_DEFINE(mkdir, const char* path, mode_t mode)
{
    PTRVALID(path);

    struct task* task = task_curr();
    char canon[256];
    vfs_mkcanon(path, task->workd, canon);

    if (canon[0] == 0) return -ENOENT;
    if (sys_access(canon, F_OK) == 0) return -EEXIST;

    return vfs_mkdir(canon, mode & ~task->umask, task->euid, task->egid);
}

SYSCALL_DEFINE(unlink, const char* pathname)
{
    PTRVALID(pathname);

    char canon[256];
    vfs_mkcanon(pathname, task_curr()->workd, canon);

    int ret = vfs_unlink(canon);

    return ret;
}

SYSCALL_DEFINE(symlink, const char* target, const char* linkpath)
{
    PTRVALID(target);
    PTRVALID(linkpath);

    char canon[256];
    vfs_mkcanon(linkpath, task_curr()->workd, canon);
    
    return vfs_symlink(target, canon);
}

SYSCALL_DEFINE(link, const char* old, const char* new)
{
    PTRVALID(old);
    PTRVALID(new);

    char oldcanon[256], newcanon[256];
    vfs_mkcanon(old, task_curr()->workd, oldcanon);
    vfs_mkcanon(new, task_curr()->workd, newcanon);

    return vfs_link(oldcanon, newcanon);
}

SYSCALL_DEFINE(rename, const char* old, const char* new)
{
    PTRVALID(old);
    PTRVALID(new);

    char oldcanon[256], newcanon[256];
    vfs_mkcanon(old, task_curr()->workd, oldcanon);
    vfs_mkcanon(new, task_curr()->workd, newcanon);

    return vfs_rename(oldcanon, newcanon);
}

SYSCALL_DEFINE(rmdir, const char* path)
{
    PTRVALID(path);

    char canon[256];
    vfs_mkcanon(path, task_curr()->workd, canon);

    return vfs_rmdir(canon);
}

SYSCALL_DEFINE(mknod, const char* path, mode_t mode, dev_t dev)
{
    PTRVALID(path);

    return vfs_mknod(path, mode, dev, task_curr()->euid, task_curr()->egid);
}
