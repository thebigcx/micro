#include <task.h>
#include <heap.h>
#include <cpu.h>

static unsigned int s_pid = 0;

static struct task* mktask()
{
    struct task* task = kmalloc(sizeof(struct task));
    task->threads = list_create();
    task->pid = s_pid++;
    return task;
}

// TODO: elf loading
struct task* task_creat(const char* file, char* argv[], char* envp[])
{
    struct task* task = mktask();

    struct thread* main = kmalloc(sizeof(struct thread));
    arch_init_thread(main, 1);

    list_push_back(&task->threads, main);

    return task;
}

struct task* task_kcreat(uintptr_t entry)
{
    struct task* task = mktask(); 

    struct thread* main = kmalloc(sizeof(struct thread));
    arch_init_thread(main, 0);

    main->regs.rip = entry;

    list_push_back(&task->threads, main);

    return task;
}
