#include <micro/sys.h>
#include <micro/wait.h>
#include <micro/sched.h>

SYSCALL_DEFINE(exit, int stat)
{
    task_exit((stat & 0xff) << 8);
    return -1; // Should be unreachable
}

// TODO: add support for thread blocking

SYSCALL_DEFINE(waitpid, int pid, int* wstatus, int options)
{
    if (options > (WNOHANG | WUNTRACED) || options < 0) return -EINVAL;
    if (pid == INT_MIN) return -ESRCH;

    struct task* task = sched_task_fromid(pid);
    if (!task || task->parent != task_curr()) return -ECHILD;

    if (!task->dead && options & WNOHANG) return 0;

    while (!task->dead) sched_yield(); // FIXME: this is a pretty poor way of mimicking thread blocking

    if (wstatus) *wstatus = task->status;

    task_delete(task);

    return pid;
}