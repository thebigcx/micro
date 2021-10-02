#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/stdlib.h>

int do_sys_open(const char* path, uint32_t flags, mode_t mode)
{
    struct task* task = task_curr();
    struct file file;

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i] && i >= 3)
        {
            int e = vfs_open_new(path, &file, flags);

            if (e == -ENOENT)
            {
                if (!(flags & O_CREAT)) return e;

                e = vfs_mknod(path, (mode & ~task->umask) | S_IFREG, 0, task->euid, task->egid);
                if (e) return e;

                vfs_open_new(path, &file, flags);
            }
            else if (flags & O_EXCL)
                return -EEXIST;
            
            if (e) return e;

            if ((flags & 3) == O_RDONLY || (flags & 3) == O_RDWR) CHECK_RPERM(file.inode);
            if ((flags & 3) == O_WRONLY || (flags & 3) == O_RDWR) CHECK_WPERM(file.inode);

            task->fds[i] = memdup(&file, sizeof(struct file));

            return i;
        }
    }

    // Out of file slots
    return -EMFILE;
}

// TODO: make opening files better and more organized
SYSCALL_DEFINE(open, const char* path, uint32_t flags, mode_t mode)
{
    PTRVALID(path);

    // Must specify an access mode
    if (!(flags & 3)) return -EINVAL;

    struct task* task = task_curr();
    char* canon = vfs_mkcanon(path, task->workd);

    return do_sys_open(canon, flags, mode);

    /*int e = vfs_resolve(canon, file, 1);

    if (!e && (flags & O_CREAT) && (flags & O_EXCL)) return -EEXIST;

    if (e == -ENOENT)
    {
        if (!(flags & O_CREAT)) return -ENOENT;
        
        if ((e = vfs_mknod(canon, (mode & ~task->umask) | S_IFREG, 0, task->euid, task->egid)))
            return e;

        vfs_resolve(canon, file, 1);
    }
    
    kfree(canon);

    if (e) return e;
    
    if ((flags & 3) == O_RDONLY || (flags & 3) == O_RDWR) CHECK_RPERM(file);
    if ((flags & 3) == O_WRONLY || (flags & 3) == O_RDWR) CHECK_WPERM(file);

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i] && i >= 3) // reserve 1, 2, 3 TODO: don't do this?
        {
            task->fds[i] = vfs_open(file, flags);
            return i;
        }
    }

    return -EMFILE;*/
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

    struct file dir;
    int e = vfs_open_new(new, &dir, O_RDONLY);

    if (e) return e;
    if (!S_ISDIR(dir.inode->mode)) return -ENOTDIR;

    strcpy(task->workd, new);

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
    int e = vfs_open_new(canon, &file, O_RDONLY);

    kfree(canon);
    if (e) return e;
    if (file.inode->uid != task_curr()->euid) return -EPERM;

    return vfs_chmod(&file, mode);
}

SYSCALL_DEFINE(fchmod, int fd, mode_t mode)
{
    FDVALID(fd);

    if (task_curr()->fds[fd]->inode->uid != task_curr()->euid) return -EPERM;
    return vfs_chmod(task_curr()->fds[fd], mode);
}

SYSCALL_DEFINE(chown, const char* pathname, uid_t uid, uid_t gid)
{
    PTRVALID(pathname);

    char* canon = vfs_mkcanon(pathname, task_curr()->workd);

    struct file file;
    int e = vfs_open_new(canon, &file, O_RDONLY);

    kfree(canon);
    if (e) return e;
    if (task_curr()->euid) return -EPERM; // Must be root TODO: capabilities like Linux

    return vfs_chown(&file, uid, gid);
}