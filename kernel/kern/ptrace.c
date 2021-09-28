#include <micro/sys.h>
#include <micro/ptrace.h>
#include <micro/task.h>
#include <micro/thread.h>
#include <micro/sched.h>
#include <micro/user.h>
#include <micro/stdlib.h>

void ptrace_attach(struct task* tracer, struct task* tracee)
{
    tracee->tracer = tracer;
}

long ptrace_getregs(struct task* task, struct user_regs* regs)
{
    memcpy(regs, &task->main->syscall_regs, sizeof(struct regs));
    return 0;
}

SYSCALL_DEFINE(ptrace, unsigned long req, pid_t pid, void* addr, void* data)
{
    if (req == PTRACE_ATTACH)
    {
        struct task* task = sched_task_fromid(pid);
        if (!task) return -ESRCH;

        ptrace_attach(task_curr(), task);
        task_send(task, SIGSTOP);
        return 0;
    }
    
    if (req == PTRACE_TRACEME)
    {
        ptrace_attach(task_curr()->parent, task_curr());
        return 0;
    }

    if (req == PTRACE_GETREGS)
    {
        PTRVALID(data);

        struct task* task = sched_task_fromid(pid);
        if (!task) return -ESRCH;

        return ptrace_getregs(task, data);
    }

    if (req == PTRACE_CONT)
    {
        struct task* task = sched_task_fromid(pid);
        if (!task) return -ESRCH;

        thread_handle_contsig(task->main);
        task->changed = 0;
        return 0;
    }

    return -EINVAL; // No such request
}