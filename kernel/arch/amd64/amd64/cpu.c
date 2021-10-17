#include <arch/cpu.h>
#include <micro/thread.h>
#include <micro/task.h>
#include <arch/reg.h>
#include <micro/stdlib.h>
#include <micro/vmmap.h>

struct cpu_info g_cpus[MAX_CPUS];
unsigned int g_cpu_cnt = 1;

struct cpu_info* cpu_curr()
{
    uintptr_t id;
    asm volatile ("cpuid" : "=b"(id) : "a"(1));
    return &g_cpus[id >> 24];
}

void cpu_set_kstack(struct cpu_info* cpu, uintptr_t kstack)
{
    cpu->tss.rsp[0] = kstack;
}

// TODO: don't need to take the cr3
extern void _switch_ctx(struct regs*, uint16_t);

void arch_switch_ctx(struct thread* thread)
{
    vm_set_currmap(thread->parent->vm_map);
    //lcr3(thread->parent->vm_map->pagemap->pml4_phys);
    _switch_ctx(&thread->regs, thread->regs.ss);
}

void arch_init_thread(struct thread* thread, int usr)
{
    thread->regs.cs = usr ? GDT_CODE3 | 3 : GDT_CODE0;
    thread->regs.ss = usr ? GDT_DATA3 | 3 : GDT_DATA0;
    thread->regs.rflags = 0x202;
}

void arch_enter_signal(struct thread* thread, int sig)
{
    struct regs r;
    memset(&r, 0, sizeof(struct regs));

    r.cs     = GDT_CODE3 | 3;
    r.ss     = GDT_DATA3 | 3;
    r.rflags = 0x202;
    r.rip    = (uintptr_t)thread->parent->signals[sig].sa_handler;
    r.rsp    = thread->sigstack; // One thread per task to handle signal

    // Push the return address onto the task's signal stack
    vm_set_currmap(thread->parent->vm_map);

    r.rsp -= 0x8;
    *(uintptr_t*)r.rsp = (uintptr_t)thread->parent->signals[sig].sa_restorer;

    _switch_ctx(&r, r.ss);
}
