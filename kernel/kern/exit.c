#include <micro/sys.h>
#include <micro/wait.h>
#include <micro/thread.h>
#include <micro/sched.h>

SYSCALL_DEFINE(exit, int stat)
{
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

SYSCALL_DEFINE(waitpid, int pid, int* wstatus, int options)
{
    if (pid !=4)
        printk("waitpid: %d, %x, %d\n", pid, wstatus, options);
    PTRVALIDNULL(wstatus);
    if (pid !=4)
        printk("waitpid: %d, %x, %d\n", pid, wstatus, options);

    if (options > (WNOHANG | WUNTRACED) || options < 0) return -EINVAL;
    if (pid == INT_MIN) return -ESRCH;

    //struct task* task = sched_task_fromid(pid);
    //if (!task || task->parent != task_curr()) return -ECHILD;

    if (pid > 0 && !sched_task_fromid(pid))
        return -ECHILD;

    if (pid == -1)
    {
        if (!task_curr()->children.size)
            return -ECHILD;

        LIST_FOREACH(&task_curr()->children)
        {
            struct task* task = node->data;

            if (has_child_changed(task))
            {
                if (wstatus) *wstatus = task->status;

                if (task->state == TASK_DEAD) // Reap the task
                    task_delete(task);

                return pid;
            }
        }
    }
    else
    {
        struct task* task = sched_task_fromid(pid);
        if (has_child_changed(task))
        {
            if (wstatus) *wstatus = task->status;

            if (task->state == TASK_DEAD) // Reap the task
                task_delete(task);

            return pid;
        }
    }

    if (!(options & WNOHANG)) // Make sure it hasn't already changed
    {
        task_curr()->waiting = pid;
        //task->waiter = thread_curr();
        thread_block(); // Block until child finished
        task_curr()->waiting = 0;
    }

    LIST_FOREACH(&task_curr()->children)
    {
        struct task* task = node->data;

        if (has_child_changed(task))
        {
            if (wstatus) *wstatus = task->status;

            if (task->state == TASK_DEAD) // Reap the task
                task_delete(task);

            return pid;
        }
    }

    if (pid == -1)
        printk("ret\n");
    return 0;
}