#include <sched.h>
#include <task.h>
#include <thread.h>
#include <cpu.h>
#include <lapic.h>
#include <debug/syslog.h>

static int ready;

void test_func()
{
    for (;;);
}

void sched_init()
{
    // TEMP
    //struct task* task = task_kcreat(test_func);
    //g_cpus[0].current = task->threads.head->data;

    for (int i = 0; i < g_cpu_cnt; i++)
    {
        dbgln("creating idle");
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
        cpu->current = cpu->idle;
        // idle thread
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
    list_foreach(&task->threads)
    {
        thread_start((struct thread*)node->data);
    }
}
