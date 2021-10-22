#include <micro/sys.h>
#include <micro/sched.h>
#include <micro/thread.h>
#include <micro/stdlib.h>
#include <micro/debug.h>

SYSCALL_DEFINE(fork)
{
    struct thread* calling = thread_curr();

    struct task* child = task_clone(task_curr(), calling);

    memcpy(&((struct thread*)child->threads.head->data)->regs, &calling->syscall_regs, sizeof(struct regs));
    ((struct thread*)child->threads.head->data)->regs.rax = 0; // return 0 to the child task

    sched_start(child);

    printk("fork() of parent=%d, child=%d\n", task_curr()->pid, child->pid);

    return child->pid;
}
