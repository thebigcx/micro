#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/stdlib.h>

// TODO: make opening files better and more organized
SYSCALL_DEFINE(open, const char* path, uint32_t flags, mode_t mode)
{
    PTRVALID(path);

    // Must specify an access mode
    if (!(flags & 3)) return -EINVAL;

    struct task* task = task_curr();

    char* canon = vfs_mkcanon(path, task->workd);
    struct file* file = kmalloc(sizeof(struct file));
    int e = vfs_resolve(canon, file, 1);

    if (!e && (flags & O_CREAT) && (flags & O_EXCL)) return -EEXIST;

    if (e == -ENOENT)
    {
        if (!(flags & O_CREAT)) return -ENOENT;
        
        if ((e = vfs_mkfile(canon, mode, task->euid, task->egid)))
            return e;

        vfs_resolve(canon, file, 1);
    }
    
    kfree(canon);

    if (e) return e;

    if (flags & O_RDONLY || flags & O_RDWR) CHECK_RPERM(file);
    if (flags & O_WRONLY || flags & O_RDWR) CHECK_WPERM(file);

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i])
        {
            task->fds[i] = vfs_open(file, flags, mode);
            return i;
        }
    }

    return -EMFILE;
}

SYSCALL_DEFINE(close, int fd)
{
    FDVALID(fd);

    struct task* task = task_curr();

    vfs_close(task->fds[fd]);
    task->fds[fd] = NULL;

    return 0;
}

SYSCALL_DEFINE(access, const char* pathname, int mode)
{
    PTRVALID(pathname);

    char* canon = vfs_mkcanon(pathname, task_curr()->workd);
    int ret = vfs_access(canon, mode);
    kfree(canon);

    return ret;
}

// TODO: add PATH_MAX macro
SYSCALL_DEFINE(chdir, const char* path)
{
    PTRVALID(path);

    struct task* task = task_curr();
    char* new = vfs_mkcanon(path, task->workd);

    struct file* dir = kmalloc(sizeof(struct file));
    int e = vfs_resolve(new, dir, 1);
    
    if (e) return e;
    if (dir->type != FL_DIR) return -ENOTDIR;

    strcpy(task->workd, new);

    kfree(dir);
    kfree(new);

    return 0;
}

SYSCALL_DEFINE(getcwd, char* buf, size_t size)
{
    PTRVALID(buf);
    
    if (size == 0) return -EINVAL;

    struct task* task = task_curr();
    if (size < strlen(task->workd) + 1) return -ERANGE;

    strcpy(buf, task->workd);

    return size;
}

SYSCALL_DEFINE(chmod, const char* pathname, mode_t mode)
{
    PTRVALID(pathname);

    char* canon = vfs_mkcanon(pathname, task_curr()->workd);
    
    struct file file;
    int e;
    if ((e = vfs_resolve(canon, &file, 1)))
        return e;

    if (file.uid != task_curr()->euid) return -EPERM;

    return vfs_chmod(&file, mode);
}

SYSCALL_DEFINE(chown, const char* pathname, uid_t uid, uid_t gid)
{
    PTRVALID(pathname);

    char* canon = vfs_mkcanon(pathname, task_curr()->workd);
    
    struct file file;
    int e;
    if ((e = vfs_resolve(canon, &file, 1)))
        return e;

    if (task_curr()->euid != 0) return -EPERM;

    return vfs_chown(&file, uid, gid);
}