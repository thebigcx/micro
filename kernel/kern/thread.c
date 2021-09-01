#include <thread.h>
#include <mmu.h>
#include <cpu.h>

void thread_start(struct thread* thread)
{
    // TODO: TEMP
    list_push_back(&g_cpus[0].threads, thread);
    list_push_back(&g_cpus[0].ready, thread);
}

struct thread* thread_creat(struct task* parent, uintptr_t entry, int usr)
{
    struct thread* thread = kmalloc(sizeof(struct thread));
    arch_init_thread(thread, usr);

    thread->parent = parent;
    thread->regs.rip = entry;

    uintptr_t kstack = mmu_kalloc(1);
    mmu_kmap(kstack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    thread->kstack = kstack + PAGE4K;

    return thread;
}

// TODO: impl
struct thread* thread_clone(struct task* parent, struct thread* src)
{
    return src;
}

struct thread* thread_curr()
{
    return cpu_curr()->current;
}