#include <micro/sched.h>
#include <micro/task.h>
#include <micro/thread.h>
#include <arch/cpu.h>
#include <arch/lapic.h>
#include <micro/debug.h>

static int ready = 0;
static struct list tasks;

void sched_init()
{
    tasks = list_create();

    idt_set_handler(IPI_SCHED, switch_task);

    for (int i = 0; i < g_cpu_cnt; i++)
    {
        printk("creating idle\n");
        g_cpus[i].idle = task_idle()->threads.head->data;
    }
    
    ready = 1;
    for(;;);
}

void switch_next()
{
    struct cpu_info* cpu = cpu_curr();
    if (TEST_LOCK(cpu->lock)) return;

    cpu->current = cpu_next_ready(cpu);
    if (cpu->current == cpu->current->parent->main)
    {
        if (cpu->current->parent->sigqueue.size)
        {
            UNLOCK(cpu->lock); // In case of task switch
            thread_handle_signals(cpu->current);
            LOCK(cpu->lock);
        }
    }

    cpu->current->state = THREAD_RUNNING;
    cpu_set_kstack(cpu, cpu->current->kstack);

    UNLOCK(cpu->lock);

    arch_switch_ctx(cpu->current);
}

void switch_task(struct regs* r)
{
    struct cpu_info* cpu = cpu_curr();
    if (TEST_LOCK(cpu->lock)) return;

    if (cpu->current)
    {
        cpu->current->regs = *r;
        if (cpu->current->state == THREAD_RUNNING)
            cpu->current->state = THREAD_READY;
    }

    UNLOCK(cpu->lock);

    switch_next();
}

void sched_tick(struct regs* r)
{
    if (!ready) return;
    lapic_send_ipi(0, IPI_SCHED, DST_OTHERS | DELIV_FIXED);
    switch_task(r);
}

void sched_start(struct task* task)
{
    list_push_back(&tasks, task);
    LIST_FOREACH(&task->threads)
    {
        thread_start((struct thread*)node->data);
    }
}

void sched_yield()
{
    sti();
    asm volatile ("int $0xfe");
}

struct task* sched_task_fromid(int id)
{
    LIST_FOREACH(&tasks)
    {
        struct task* task = node->data;

        if (task->id == id) return task;
    }

    return NULL;
}