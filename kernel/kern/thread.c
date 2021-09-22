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
    //list_push_back(&g_cpus[0].threads, thread);
    //thread->state = THREAD_READY;
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

void thread_handle_signals(struct thread* thread)
{
    int* sigptr = list_dequeue(&thread->parent->sigqueue);
    int sig = *sigptr;
    kfree(sigptr);

    uintptr_t handler = thread->parent->signals[sig];

    if (!handler)
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
                thread->parent->state = TASK_STOPPED; // TODO: make this better
                thread->parent->status = (sig << 8) | 0x7f; // TODO: not actually an exit code, just a status
                thread->parent->changed = 1;
                switch_next();
                break;
                
            case SIGDEF_CONT:
                thread->parent->state = TASK_RUNNING;
                thread->parent->changed = 1;
                thread->parent->status = 0xffff;
                break;
        }
    }
    else
    {
        // TODO: set up signal handler stuff
    }
}