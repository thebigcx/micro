#include <micro/signal.h>
#include <micro/sys.h>
#include <micro/sched.h>
#include <micro/stdlib.h>
#include <micro/thread.h>

#include <arch/cpu.h>

SYSCALL_DEFINE(kill, int pid, int sig)
{
    if (sig < 0 || sig > 32) return -EINVAL;

    // TODO: impl proper
    struct task* task = sched_task_fromid(pid);
    if (!task) return -ESRCH;
    task_send(task, sig);

    return 0;
}

SYSCALL_DEFINE(sigaction, int signum, const struct sigaction* act,
               struct sigaction* oldact)
{
    if (signum < 0 || signum >= 32
     || signum == SIGKILL || signum == SIGSTOP) return -EINVAL;

    struct task* task = task_curr();

    if (oldact)
    {
        PTRVALID(oldact);
        memcpy(oldact, &task->signals[signum], sizeof(struct sigaction));
    }

    if (act)
    {
        PTRVALID(act);
        task->signals[signum] = *act;
    }

    return 0;
}

SYSCALL_DEFINE(sigprocmask, int how, const sigset_t* set, sigset_t* oldset)
{
    PTRVALIDNULL(set);
    PTRVALIDNULL(oldset);

    struct thread* thread = thread_curr();

    if (oldset)
        *oldset = thread->sigmask;

    if (set)
    {
        switch (how)
        {
            case SIG_BLOCK:
                thread->sigmask |= *set;
                break;
            case SIG_UNBLOCK:
                thread->sigmask &= ~(*set);
                break;
            case SIG_SETMASK:
                thread->sigmask = *set;
                break;
            default:
                return -EINVAL;
        }
    }

    return 0;
}

SYSCALL_DEFINE(sigreturn)
{
    sched_yield();
    __builtin_unreachable();
}
