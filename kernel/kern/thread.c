#include <micro/thread.h>
#include <micro/task.h>
#include <micro/sched.h>
#include <arch/mmu.h>
#include <arch/cpu.h>
#include <micro/stdlib.h>
#include <micro/debug.h>
#include <micro/heap.h>
#include <micro/sys.h>

static void init_thread_meta(struct thread* thread, struct task* parent)
{
    thread->parent = parent;
    thread->state = THREAD_READY;

    uintptr_t kstack = mmu_kalloc(1);
    mmu_kmap(kstack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    thread->kstack = kstack + PAGE4K;
    
    thread->sigqueue = list_create();
    thread->sigmask  = 0;

    struct vm_area* sigstack = vm_map_anon(parent->vm_map, 0, PAGE4K, 0);
    vm_map_anon_alloc(parent->vm_map, sigstack, sigstack->base, sigstack->end - sigstack->base);
    thread->sigstack = sigstack->end;
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

void thread_free(struct thread* thread)
{
    thread->state = THREAD_DEAD;
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
    while (thread->sigqueue.size)
    {
        struct signal* sig = list_dequeue(&thread->sigqueue);
        int signo = sig->num;
        kfree(sig);
        
        printk("pid=%d handling signal %d\n", thread->parent->pid, signo);

        if (thread->parent->tracer)
            handle_stopsig(thread, signo);

        // Ignore signal if in the signal mask
        if (signo != SIGKILL && signo != SIGSTOP && (thread->sigmask & (1 << signo)))
            return;

        sighandler_t handler = thread->parent->signals[signo].sa_handler;

        if (!handler || handler == SIG_DFL) // SIG_DFL is 0 anyway
        {
            switch (defaults[signo])
            {
                case SIGDEF_TERM:
                    task_exit(signo);
                    break;

                case SIGDEF_CORE:
                    task_exit(signo | 0x80); // Core flag 0x80
                    break;

                case SIGDEF_STOP:
                    handle_stopsig(thread, signo);
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
            arch_enter_signal(thread, signo);
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

SYSCALL_DEFINE(tkill, pid_t tid, int sig)
{
    return 0;
}
