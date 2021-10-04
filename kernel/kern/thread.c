#include <micro/thread.h>
#include <micro/task.h>
#include <micro/sched.h>
#include <arch/mmu.h>
#include <arch/cpu.h>
#include <micro/stdlib.h>
#include <micro/debug.h>
#include <micro/heap.h>

static void init_thread_meta(struct thread* thread, struct task* parent)
{
    thread->parent = parent;
    thread->state = THREAD_READY;

    uintptr_t kstack = mmu_kalloc(1);
    mmu_kmap(kstack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    thread->kstack = kstack + PAGE4K;
}

struct thread* thread_creat(struct task* parent, uintptr_t entry, int usr)
{
    struct thread* thread = kcalloc(sizeof(struct thread));
    
    arch_init_thread(thread, usr);

    init_thread_meta(thread, parent);
    thread->regs.rip = entry;

    return thread;
}

// TODO: impl
struct thread* thread_clone(struct task* parent, struct thread* src)
{
    struct thread* t = memdup(src, sizeof(struct thread));
    init_thread_meta(t, parent);
    return t;
}

struct thread* thread_curr()
{
    return cpu_curr()->current;
}

// Default actions
#define SIGDEF_TERM 0
#define SIGDEF_IGN  1
#define SIGDEF_CORE 2
#define SIGDEF_STOP 3
#define SIGDEF_CONT 4

// Signal Defaults 
static int defaults[] =
{
    [SIGABRT  ] = SIGDEF_CORE,
    [SIGALRM  ] = SIGDEF_TERM,
    [SIGBUS   ] = SIGDEF_CORE,
    [SIGCHLD  ] = SIGDEF_IGN,
    [SIGCONT  ] = SIGDEF_CONT,
    [SIGFPE   ] = SIGDEF_CORE,
    [SIGHUP   ] = SIGDEF_TERM,
    [SIGILL   ] = SIGDEF_CORE,
    [SIGINT   ] = SIGDEF_TERM,
    [SIGKILL  ] = SIGDEF_TERM,
    [SIGPIPE  ] = SIGDEF_TERM,
    [SIGPOLL  ] = SIGDEF_TERM,
    [SIGPROF  ] = SIGDEF_TERM,
    [SIGQUIT  ] = SIGDEF_CORE,
    [SIGSEGV  ] = SIGDEF_CORE,
    [SIGSTOP  ] = SIGDEF_STOP,
    [SIGTSTP  ] = SIGDEF_STOP,
    [SIGSYS   ] = SIGDEF_CORE,
    [SIGTERM  ] = SIGDEF_TERM,
    [SIGTRAP  ] = SIGDEF_CORE,
    [SIGTTIN  ] = SIGDEF_STOP,
    [SIGTTOU  ] = SIGDEF_STOP,
    [SIGURG   ] = SIGDEF_IGN,
    [SIGUSR1  ] = SIGDEF_TERM,
    [SIGUSR2  ] = SIGDEF_TERM,
    [SIGVTALRM] = SIGDEF_TERM,
    [SIGXCPU  ] = SIGDEF_CORE,
    [SIGXFSZ  ] = SIGDEF_CORE,
    [SIGWINCH ] = SIGDEF_IGN
};

static void handle_stopsig(struct thread* thread, int sig)
{
    task_change(thread->parent, TASK_STOPPED);
    thread->parent->status = (sig << 8) | 0x7f;

    switch_next();
}

void thread_handle_contsig(struct thread* thread)
{
    thread->parent->status = 0xffff;
    task_change(thread->parent, TASK_RUNNING);
    sched_spawnthread(thread);
}

// TODO: scan threads for a thread which can handle the signal (not in the sigmask)
// (don't necessarily use the main thread for signal handling)

void thread_handle_signals(struct thread* thread)
{
    while (thread->parent->sigqueue.size)
    {
        int* sigptr = list_dequeue(&thread->parent->sigqueue);
        int sig = *sigptr;
        kfree(sigptr);
        printk("pid=%d handling signal %d\n", thread->parent->pid, sig);

        if (thread->parent->tracer)
            handle_stopsig(thread, sig);

        // Ignore signal if in the signal mask
        if (sig != SIGKILL && sig != SIGSTOP && (thread->parent->sigmask & (1 << sig)))
            return;

        uintptr_t handler = thread->parent->signals[sig].sa_handler;
        printk("%x\n", handler);

        if (!handler || handler == SIG_DFL) // SIG_DFL is 0 anyway
        {
            switch (defaults[sig])
            {
                case SIGDEF_TERM:
                    task_exit(sig);
                    break;

                case SIGDEF_CORE:
                    task_exit(sig | 0x80); // Core flag 0x80
                    break;

                case SIGDEF_STOP:
                    handle_stopsig(thread, sig);
                    break;
                    
                case SIGDEF_CONT:
                    thread_handle_contsig(thread);
                    break;
            }
        }
        else if (handler == SIG_IGN)
        {
            /* Do nothing */
        }
        else
        {
            arch_enter_signal(thread, sig);
        }
    }
}

void thread_block()
{
    thread_curr()->state = THREAD_BLOCKED;
    sched_yield();
}

void thread_unblock(struct thread* thread)
{
    sched_spawnthread(thread);
}