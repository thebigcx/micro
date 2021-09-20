#include <micro/sys.h>
#include <micro/ptrace.h>
#include <micro/task.h>
#include <micro/thread.h>
#include <micro/sched.h>
#include <micro/user.h>
#include <micro/stdlib.h>

void ptrace_attach(struct task* tracer, struct task* tracee)
{
    return;
}

long ptrace_getregs(struct task* task, struct user_regs* regs)
{
    memcpy(regs, &task->main->regs, sizeof(struct regs));
    return 0;
}

SYSCALL_DEFINE(ptrace, enum ptrace_req req, pid_t pid, void* addr, void* data)
{
    if (req < 0 || req >= PTRACE_REQ_CNT) return -EINVAL;

    struct task* task = sched_task_fromid(pid);
    if (!task) return -ESRCH;

    if (req == PTRACE_ATTACH)
    {
        ptrace_attach(task_curr(), task);
        task_send(task, SIGSTOP);
        return 0;
    }
    
    if (req == PTRACE_TRACEME)
    {
        ptrace_attach(task_curr()->parent, task_curr());
        return 0;
    }

    if (req == PTRACE_GETFREGS)
    {
        PTRVALID(data);
        return ptrace_getregs(task, data);
    }
}