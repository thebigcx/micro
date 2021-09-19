#include <micro/sys.h>
#include <micro/vfs.h>

SYSCALL_DEFINE(dup, int oldfd)
{
    FDVALID(oldfd);
    
    struct task* task = task_curr();

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (!task->fds[i])
        {
            task->fds[i] = oldfd;
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