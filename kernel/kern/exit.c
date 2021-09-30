#include <micro/sys.h>
#include <micro/wait.h>
#include <micro/thread.h>
#include <micro/sched.h>

SYSCALL_DEFINE(exit, int stat)
{
    task_exit((stat & 0xff) << 8);
    return 0; // Should be unreachable
}

// TODO: add support for thread blocking

SYSCALL_DEFINE(waitpid, int pid, int* wstatus, int options)
{
    if (pid !=4)
        printk("waitpid: %d, %x, %d\n", pid, wstatus, options);
    PTRVALIDNULL(wstatus);

    if (options > (WNOHANG | WUNTRACED) || options < 0) return -EINVAL;
    if (pid == INT_MIN) return -ESRCH;

    struct task* task = sched_task_fromid(pid);
    if (!task || task->parent != task_curr()) return -ECHILD;

    if (!(options & WNOHANG) && !task->changed) // Make sure it hasn't already changed
    {
        task->waiter = thread_curr();
        thread_block(); // Block until child finished
    }
    else if (options & WNOHANG && !task->changed)
    {
        return 0;
    }

    if (wstatus)
        *wstatus = task->status;

    if (task->state == TASK_DEAD) // Reap the task
        task_delete(task);

    return pid;
}