#include <micro/signal.h>
#include <micro/sys.h>
#include <micro/sched.h>

#include <arch/cpu.h>

SYSCALL_DEFINE(kill, int pid, int sig)
{
    if (sig < 0 || sig > 32) return -EINVAL;

    // TODO: impl proper
    struct task* task = sched_task_fromid(pid);
    if (!task) return -ESRCH;
    task_send(task, sig);

    if (task_curr() == task) sched_yield();

    return 0;
}

SYSCALL_DEFINE(sigaction, int signum, const struct sigaction* act,
               struct sigaction* oldact)
{
    // TODO: proper implementation
    task_curr()->signals[signum] = *act;

    return 0;
}

SYSCALL_DEFINE(sigprocmask, int how, const sigset_t* set, sigset_t* oldset)
{
    PTRVALIDNULL(set);
    PTRVALIDNULL(oldset);

    struct task* task = task_curr();

    if (oldset)
        *oldset = task->sigmask;

    if (set)
    {
        switch (how)
        {
            case SIG_BLOCK:
                task->sigmask |= *set;
                break;
            case SIG_UNBLOCK:
                task->sigmask &= ~(*set);
                break;
            case SIG_SETMASK:
                task->sigmask = *set;
                break;
            default:
                return -EINVAL;
        }
    }

    return 0;
}

SYSCALL_DEFINE(sigreturn)
{
    return -1; // Won't return
}