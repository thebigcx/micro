#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/heap.h>

SYSCALL_DEFINE(mkdir, const char* path, mode_t mode)
{
    PTRVALID(path);

    struct task* task = task_curr();
    char* canon = vfs_mkcanon(path, task->workd);

    if (canon[0] == 0) return -ENOENT;
    if (sys_access(canon, F_OK) == 0) return -EEXIST;

    return vfs_mkdir(canon, mode, task->euid, task->egid);
}

SYSCALL_DEFINE(unlink, const char* pathname)
{
    PTRVALID(pathname);

    char* canon = vfs_mkcanon(pathname, task_curr()->workd);

    int ret = vfs_unlink(canon);

    kfree(canon);
    return ret;
}

SYSCALL_DEFINE(symlink, const char* target, const char* linkpath)
{
    PTRVALID(target);
    PTRVALID(linkpath);

    char* canon = vfs_mkcanon(linkpath, task_curr()->workd);
    
    int ret = vfs_symlink(target, canon);

    kfree(canon);
    return ret;
}

SYSCALL_DEFINE(link, const char* old, const char* new)
{
    PTRVALID(old);
    PTRVALID(new);

    char* oldcanon = vfs_mkcanon(old, task_curr()->workd);
    char* newcanon = vfs_mkcanon(new, task_curr()->workd);

    int ret = vfs_link(oldcanon, newcanon);

    kfree(oldcanon);
    kfree(newcanon);
    return ret;
}

SYSCALL_DEFINE(rename, const char* old, const char* new)
{
    PTRVALID(old);
    PTRVALID(new);

    char* oldcanon = vfs_mkcanon(old, task_curr()->workd);
    char* newcanon = vfs_mkcanon(new, task_curr()->workd);

    int ret = vfs_rename(oldcanon, newcanon);

    kfree(oldcanon);
    kfree(newcanon);
    return ret;
}

SYSCALL_DEFINE(rmdir, const char* path)
{
    PTRVALID(path);

    char* canon = vfs_mkcanon(path, task_curr()->workd);

    int ret = vfs_rmdir(canon);

    kfree(canon);
    return ret;
}