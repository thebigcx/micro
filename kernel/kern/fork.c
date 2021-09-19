#include <micro/sys.h>
#include <micro/sched.h>
#include <micro/thread.h>
#include <micro/stdlib.h>

SYSCALL_DEFINE(fork)
{
    struct thread* calling = thread_curr();

    struct task* child = task_clone(task_curr(), calling);

    memcpy(&((struct thread*)child->threads.head->data)->regs, &calling->syscall_regs, sizeof(struct regs));
    ((struct thread*)child->threads.head->data)->regs.rax = 0; // return 0 to the child task

    sched_start(child);

    return child->id;
}