#include <micro/thread.h>
#include <micro/task.h>
#include <micro/sched.h>
#include <arch/mmu.h>
#include <arch/cpu.h>
#include <micro/stdlib.h>
#include <micro/debug.h>
#include <micro/heap.h>

void thread_start(struct thread* thread)
{
    // TODO: TEMP SMP
    list_push_back(&g_cpus[0].threads, thread);
    thread->state = THREAD_READY;
}

struct thread* thread_creat(struct task* parent, uintptr_t entry, int usr)
{
    struct thread* thread = kmalloc(sizeof(struct thread));
    memset(&thread->regs, 0, sizeof(struct regs));
    
    arch_init_thread(thread, usr);

    thread->parent = parent;
    thread->regs.rip = entry;
    thread->state = THREAD_READY;

    uintptr_t kstack = mmu_kalloc(1);
    mmu_kmap(kstack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    thread->kstack = kstack + PAGE4K;

    return thread;
}

// TODO: impl
struct thread* thread_clone(struct task* parent, struct thread* src)
{
    struct thread* thread = kmalloc(sizeof(struct thread));
    memcpy(thread, src, sizeof(struct thread));
    
    thread->parent = parent;
    thread->state = THREAD_READY;

    uintptr_t kstack = mmu_kalloc(1);
    mmu_kmap(kstack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    thread->kstack = kstack + PAGE4K;

    return thread;
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
    [SIGABRT  ] = SIGDEF_TERM,
    [SIGALRM  ] = SIGDEF_TERM, 
    [SIGBUS   ] = SIGDEF_TERM,
    [SIGCHLD  ] = SIGDEF_IGN,
    [SIGCONT  ] = SIGDEF_TERM,
    [SIGFPE   ] = SIGDEF_TERM,
    [SIGHUP   ] = SIGDEF_TERM,
    [SIGILL   ] = SIGDEF_TERM,
    [SIGINFO  ] = SIGDEF_TERM,
    [SIGINT   ] = SIGDEF_TERM,
    [SIGKILL  ] = SIGDEF_TERM,
    [SIGPIPE  ] = SIGDEF_TERM,
    [SIGPOLL  ] = SIGDEF_TERM,
    [SIGPROF  ] = SIGDEF_TERM,
    [SIGQUIT  ] = SIGDEF_TERM,
    [SIGSEGV  ] = SIGDEF_TERM,
    [SIGSTOP  ] = SIGDEF_TERM,
    [SIGTSTP  ] = SIGDEF_TERM,
    [SIGSYS   ] = SIGDEF_TERM,
    [SIGTERM  ] = SIGDEF_TERM,
    [SIGTRAP  ] = SIGDEF_TERM,
    [SIGTTIN  ] = SIGDEF_TERM,
    [SIGTTOU  ] = SIGDEF_TERM,
    [SIGURG   ] = SIGDEF_TERM,
    [SIGUSR1  ] = SIGDEF_TERM,
    [SIGUSR2  ] = SIGDEF_TERM,
    [SIGVTALRM] = SIGDEF_TERM,
    [SIGXCPU  ] = SIGDEF_TERM,
    [SIGXFSZ  ] = SIGDEF_TERM
};

void thread_handle_signals(struct thread* thread)
{
    int* sigptr = list_dequeue(&thread->parent->sigqueue);
    int sig = *sigptr;
    kfree(sigptr);

    if (sig == SIGCHLD)
    {
        thread->parent->waiting = 0;
    }

    uintptr_t handler = thread->parent->signals[sig];

    if (!handler)
    {
        switch (defaults[sig])
        {
            case SIGDEF_TERM:
                printk("Terminating from signal %d\n", sig);
                task_exit(sig + 128);
                break;

            case SIGDEF_CORE:
                break;

            case SIGDEF_STOP:
                break;
                
            case SIGDEF_CONT:
                break;
        }
    }
    else
    {
        // TODO: set up signal handler stuff
    }
}