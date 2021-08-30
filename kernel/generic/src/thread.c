#include <thread.h>
#include <mmu.h>

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

    //uintptr_t stack = mmu_kalloc(1);
    //mmu_kmap(stack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    uintptr_t kstack = mmu_kalloc(1);
    mmu_kmap(kstack, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    //thread->regs.rsp = stack + PAGE4K;
    //thread->regs.rbp = stack + PAGE4K;
    thread->kstack = kstack + PAGE4K;

    return thread;
}
