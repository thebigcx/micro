#include <sched.h>
#include <task.h>
#include <thread.h>
#include <cpu.h>
#include <lapic.h>
#include <debug/syslog.h>

static int ready = 0;

void sched_init()
{
    idt_set_handler(IPI_SCHED, switch_next);

    for (int i = 0; i < g_cpu_cnt; i++)
    {
        printk("creating idle\n");
        g_cpus[i].idle = task_idle()->threads.head->data;
    }
    
    ready = 1;
    for(;;);
}

void switch_next(struct regs* r)
{
    struct cpu_info* cpu = cpu_curr();
    if (TEST_LOCK(cpu->lock)) return;

    if (cpu->current)
    {
        cpu->current->regs = *r;
        if (cpu->current->state == THREAD_RUNNING)
        {
            // add back
            list_push_back(&cpu->ready, cpu->current);
        }
    }

    if (!cpu->ready.size)
    {
        cpu->current = cpu->idle; // idle thread
    }
    else
    {
        cpu->current = list_pop_front(&cpu->ready);
    }

    cpu->current->state = THREAD_RUNNING;

    cpu_set_kstack(cpu, cpu->current->kstack);

    UNLOCK(cpu->lock);

    arch_switch_ctx(cpu->current);
}

void sched_tick(struct regs* r)
{
    if (!ready) return;
    lapic_send_ipi(0, IPI_SCHED, DST_OTHERS | DELIV_FIXED);
    switch_next(r);
}

void sched_start(struct task* task)
{
    LIST_FOREACH(&task->threads)
    {
        thread_start((struct thread*)node->data);
    }
}

void sched_yield()
{
    asm volatile ("int $0xfe");
}