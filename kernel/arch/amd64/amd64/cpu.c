#include <cpu.h>
#include <thread.h>
#include <task.h>
#include <reg.h>

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

struct thread* cpu_next_ready(struct cpu_info* cpu)
{
    // Make sure we haven't looped back to the start
    uintptr_t i = cpu->threads.size;

    do
    {
        if (!cpu->threads.size || !i)
        {
            cpu->current = cpu->idle;
            break;
        }
        else
        {
            cpu->current = list_pop_front(&cpu->threads);
            list_push_back(&cpu->threads, cpu->current);
            i--;
        }

    } while (cpu->current->state != THREAD_READY);

    return cpu->current;
}

void _switch_ctx(struct regs*, uintptr_t, uint16_t);

void arch_switch_ctx(struct thread* thread)
{
    _switch_ctx(&thread->regs, thread->parent->vm_map->pml4_phys, thread->regs.ss);
}

void arch_init_thread(struct thread* thread, int usr)
{
    thread->regs.cs = usr ? GDT_CODE3 | 3 : GDT_CODE0;
    thread->regs.ss = usr ? GDT_DATA3 | 3 : GDT_DATA0;
    thread->regs.rflags = 0x202;
}
