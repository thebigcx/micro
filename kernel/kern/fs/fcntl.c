#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>

SYSCALL_DEFINE(fcntl, int fdno, int cmd, void* arg)
{
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    switch (cmd)
    {
        case F_GETFL:
            return fd->flags;
    }


    printk("warning: call to fcntl(fd=%d, cmd=%d, arg=%x) not implemented!\n", fd, cmd, arg);
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