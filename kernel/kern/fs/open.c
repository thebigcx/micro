#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/stdlib.h>
#include <micro/try.h>

int do_sys_open(const char* path, uint32_t flags, mode_t mode)
{
    struct task* task = task_curr();
    struct file file;

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i] && i >= 3)
        {
            int e = vfs_open(path, &file, flags);

            if (e == -ENOENT)
            {
                if (!(flags & O_CREAT)) return e;

                e = vfs_mknod(path, (mode & ~task->umask) | S_IFREG, 0, task->euid, task->egid);
                if (e) return e;

                vfs_open(path, &file, flags);
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
    if ((flags & 3) > 2) return -EINVAL;

    struct task* task = task_curr();
    char canon[256];
    vfs_mkcanon(path, task->workd, canon);

    return do_sys_open(canon, flags, mode);
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

    char canon[256];
    vfs_mkcanon(pathname, task_curr()->workd, canon);
    return vfs_access(canon, mode);
}

// TODO: add PATH_MAX macro
SYSCALL_DEFINE(chdir, const char* path)
{
    PTRVALID(path);

    struct task* task = task_curr();
    char canon[256];
    vfs_mkcanon(path, task->workd, canon);
    
    struct file dir;
    TRY(vfs_open(canon, &dir, O_RDONLY));
    
    if (!S_ISDIR(dir.inode->mode)) return -ENOTDIR;

    strcpy(task->workd, canon);
    return 0;
}

SYSCALL_DEFINE(getcwd, char* buf, size_t size)
{
    PTRVALID(buf);
    
    if (size == 0) return -EINVAL;

    struct task* task = task_curr();
    if (size < strlen(task->workd) + 1) return -ERANGE;

    strcpy(buf, task->workd);

    return (uintptr_t)buf;
}

SYSCALL_DEFINE(chmod, const char* pathname, mode_t mode)
{
    PTRVALID(pathname);

    char canon[256];
    vfs_mkcanon(pathname, task_curr()->workd, canon);
    
    struct file file;
    TRY(vfs_open(canon, &file, O_RDONLY));
    
    if (task_curr()->euid
        && task_curr()->euid && file.inode->uid != task_curr()->euid) return -EPERM;

    return vfs_chmod(&file, mode);
}

// Change file permissions of file referred to by 'fd'
SYSCALL_DEFINE(fchmod, int fd, mode_t mode)
{
    FDVALID(fd);

    // Must be the owner or root
    if (task_curr()->euid
        && task_curr()->fds[fd]->inode->uid != task_curr()->euid) return -EPERM;
    
    return vfs_chmod(task_curr()->fds[fd], mode);
}

int do_chown(const char* path, uid_t uid, gid_t gid, int symlinks)
{
    char canon[256];
    vfs_mkcanon(path, task_curr()->workd, canon);

    uint32_t mode = O_RDONLY;
    if (!symlinks) mode |= O_PATH | O_NOFOLLOW;

    struct file file;
    TRY(vfs_open(canon, &file, mode));
    
    // Must be root
    if (task_curr()->euid) return -EPERM;
    
    return vfs_chown(&file, uid, gid);
}

// Change ownership of file
SYSCALL_DEFINE(chown, const char* path, uid_t uid, uid_t gid)
{
    PTRVALID(path);
    return do_chown(path, uid, gid, 1);
}

// Change ownership of file referred to by 'fd'
SYSCALL_DEFINE(fchown, int fd, uid_t owner, gid_t group)
{
    FDVALID(fd);
    if (task_curr()->euid) return -EPERM;
    
    return vfs_chown(task_curr()->fds[fd], owner, group);
}

// Change ownership of file without following symlinks
SYSCALL_DEFINE(lchown, const char* path, uid_t owner, gid_t group)
{
    PTRVALID(path);
    return do_chown(path, owner, group, 0);
}

SYSCALL_DEFINE(faccessat2, int dirfd, const char* path, int mode, int flags)
{
    printk("faccessat2(%d, %s, %d, %d)\n", dirfd, path, mode, flags);
    return 0;
}
