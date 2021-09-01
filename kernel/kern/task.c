#include <task.h>
#include <heap.h>
#include <cpu.h>
#include <thread.h>
#include <debug/syslog.h>
#include <binfmt.h>
#include <vfs.h>

static unsigned int s_id = 0;

static struct task* mktask()
{
    struct task* task = kmalloc(sizeof(struct task));

    task->threads = list_create();
    task->fds = list_create();
    task->id = s_id++;
    task->vm_map = mmu_create_vmmap();

    return task;
}

static void idle()
{
    for(;;);
}

struct task* task_idle()
{
    return task_kcreat(idle);
}

// TODO: elf loading
struct task* task_creat(const void* buffer, char* argv[], char* envp[])
{
    struct task* task = mktask();

    uintptr_t entry = elf_load(task, buffer);
    struct thread* main = thread_creat(task, entry, 1);
    
    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    mmu_map(task->vm_map, stack - 0x1000, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    main->regs.rsp = stack;
    main->regs.rbp = stack;

    list_push_back(&task->threads, main);

    return task;
}

struct task* task_kcreat(uintptr_t entry)
{
    struct task* task = mktask(); 

    struct thread* main = thread_creat(task, entry, 0);

    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    mmu_map(task->vm_map, stack - 0x1000, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    main->regs.rsp = stack;
    main->regs.rbp = stack;

    list_push_back(&task->threads, main);

    return task;
}

struct task* task_clone(const struct task* src, struct thread* calling)
{
    struct task* task = kmalloc(sizeof(struct task));

    task->id = 0;
    task->vm_map = mmu_clone_vmmap(src->vm_map);

    struct thread* main = thread_clone(task, calling);
    list_push_back(&task->threads, main);

    LIST_FOREACH(&src->fds)
    {
        struct fd* fd = node->data;
        list_push_back(&task->fds, vfs_open(fd->filp));
    }

    return task;
}

struct task* task_curr()
{
    return thread_curr()->parent;
}