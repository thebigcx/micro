#include <micro/sched.h>
#include <micro/task.h>
#include <micro/thread.h>
#include <arch/cpu.h>
#include <arch/lapic.h>
#include <micro/debug.h>
#include <arch/cpu.h>

static int ready = 0;
static struct list tasks;
static struct list ready_queue;

static struct thread* next_ready(struct cpu_info* cpu)
{
    // Make sure we haven't looped back to the start
    /*uintptr_t i = threads.size;

    do
    {
        if (!threads.size || !i)
        {
            cpu->current = cpu->idle;
            break;
        }
        else
        {
            cpu->current = list_dequeue(&threads);
            list_push_back(&threads, cpu->current);
            i--;
        }

    } while (cpu->current->state != THREAD_READY);

    return cpu->current;*/
    if (!ready_queue.size)
        return cpu->idle;

    struct thread* thread = list_dequeue(&ready_queue);
    if (thread->state == THREAD_RUNNING)
    {
        list_push_back(&ready_queue, thread);
        return cpu->idle;
    }

    return thread;
}

void sched_init()
{
    tasks = list_create();
    ready_queue = list_create();

    idt_set_handler(IPI_SCHED, switch_task);

    for (unsigned int i = 0; i < g_cpu_cnt; i++)
    {
        printk("creating idle\n");
        g_cpus[i].idle = task_idle()->threads.head->data;
        g_cpus[i].idle->state = THREAD_READY;
    }

    // /init process
    sched_start(task_init_creat());
    
    ready = 1;
    for(;;);
}

void switch_next()
{
    struct cpu_info* cpu = cpu_curr();
    if (TEST_LOCK(cpu->lock)) return;

    //cpu->current = cpu_next_ready(cpu);
    do
    {
        cpu->current = next_ready(cpu);
    } while (cpu->current->state != THREAD_READY);

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
        {
            cpu->current->state = THREAD_READY;
            list_push_back(&ready_queue, cpu->current);
        }
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
        //thread_start((struct thread*)node->data);
        struct thread* thread = node->data;
        thread->state = THREAD_READY;
        list_push_back(&ready_queue, thread);
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

        if (task->id == (unsigned int)id) return task;
    }

    return NULL;
}