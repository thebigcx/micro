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