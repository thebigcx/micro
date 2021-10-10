#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/debug.h>

SYSCALL_DEFINE(fcntl, int fdno, int cmd, unsigned long arg)
{
    FDVALID(fdno);

    struct file* fd = task_curr()->fds[fdno];

    switch (cmd)
    {
        case F_GETFD:
            printk("warning: F_GETFD returning 0\n");
            return 0;
        case F_GETFL:
            return fd->flags;
        case F_SETFD:
            //fd->flags = arg;
            return 0;
    }

    printk("warning: call to fcntl(fd=%d, cmd=%d, arg=%x) not implemented!\n", fdno, cmd, arg);
    return -EINVAL;
}

SYSCALL_DEFINE(dup, int oldfd)
{
    FDVALID(oldfd);
    
    struct task* task = task_curr();

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i])
        {
            task->fds[i] = task->fds[oldfd];
            return i;
        }
    }

    return -EMFILE;
}

SYSCALL_DEFINE(dup2, int oldfd, int newfd)
{
    FDVALID(oldfd);
    FDRANGEVALID(newfd);

    struct task* task = task_curr();
    
    task->fds[newfd] = task->fds[oldfd];
    return newfd;
}
