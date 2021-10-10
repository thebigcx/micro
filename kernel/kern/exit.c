#include <micro/sys.h>
#include <micro/wait.h>
#include <micro/thread.h>
#include <micro/sched.h>
#include <micro/debug.h>

SYSCALL_DEFINE(exit, int stat)
{
    printk("exit() of pid=%d\n", task_curr()->pid);
    task_exit((stat & 0xff) << 8);
    return 0; // Should be unreachable
}

int has_child_changed(struct task* task)
{
    // Terminated
    if (task->state == TASK_DEAD)
        return 1;

    // Stopped
    if (task->state == TASK_STOPPED && !task->was_stop)
    {
        task->was_stop = 1;
        return 1;
    }

    // Continued
    if (task->state == TASK_RUNNING && task->was_stop)
    {
        task->was_stop = 0;
        return 1;
    }

    return 0;
}

static int wait_any_child(int* wstatus, int options)
{
    LIST_FOREACH(&task_curr()->children)
    {
        struct task* task = node->data;

        if (has_child_changed(task))
        {
            if (wstatus) *wstatus = task->status;
            pid_t ret = task->pid;

            if (task->state == TASK_DEAD) // Reap the task
                task_delete(task);

            return ret;
        }
    }

    if (options & WNOHANG) return 0;

    task_curr()->waiting = -1;
    thread_block(); // Block until child finished
    task_curr()->waiting = 0;

    // TODO: this is a race condition, another child could have exited in the time it took to determine the exited child
    LIST_FOREACH(&task_curr()->children)
    {
        struct task* task = node->data;

        if (has_child_changed(task))
        {
            if (wstatus) *wstatus = task->status;
            pid_t ret = task->pid;

            if (task->state == TASK_DEAD) // Reap the task
                task_delete(task);

            return ret;
        }
    }

    // Shouldn't be here
    return 0;
}

int wait_child(struct task* child, int* wstatus, int options)
{
    if (has_child_changed(child))
    {
        if (wstatus) *wstatus = child->status;
        pid_t ret = child->pid; // Going to get deleted

        if (child->state == TASK_DEAD) // Reap the task
            task_delete(child);

        return ret;
    }

    if (options & WNOHANG)
        return 0;

    task_curr()->waiting = child->pid;
    thread_block(); // Block until child finished
    task_curr()->waiting = 0;

    // TODO: another race condition
    return child->pid;
}

SYSCALL_DEFINE(waitpid, int pid, int* wstatus, int options)
{
    if (pid != 4)
        printk("waitpid(%d) of pid=%d\n", pid, task_curr()->pid);
    PTRVALIDNULL(wstatus);

    if (options > (WNOHANG | WUNTRACED) || options < 0) return -EINVAL;

    if (pid > 0)
    {
        struct task* child = sched_task_fromid(pid);
        if (!child) return -ECHILD;
        return wait_child(child, wstatus, options);
    }
    else if (pid == -1)
    {
        return wait_any_child(wstatus, options);
    }
    else
    {
        printk("kernel: warning: waitpid(pid=%d) not implemented!\n");
        return 0;
    }

    return 0;
}

SYSCALL_DEFINE(wait4, pid_t pid, int* wstatus, int options, struct rusage* rusage)
{
    return sys_waitpid(pid, wstatus, options);
}
