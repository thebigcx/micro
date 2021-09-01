#include <task.h>
#include <heap.h>
#include <cpu.h>
#include <thread.h>
#include <debug/syslog.h>
#include <binfmt.h>
#include <vfs.h>

static unsigned int s_id = 1;

static struct task* mktask(struct task* parent)
{
    struct task* task = kmalloc(sizeof(struct task));

    task->threads = list_create();
    task->fds = list_create();
    task->id = s_id++;
    task->vm_map = mmu_create_vmmap();
    task->children = list_create();
    task->parent = parent;

    return task;
}

static void idle()
{
    for(;;);
}

struct task* task_idle()
{
    return task_kcreat(NULL, idle);
}

// TODO: elf loading
struct task* task_creat(struct task* parent, const void* buffer, char* argv[], char* envp[])
{
    struct task* task = mktask(parent);

    uintptr_t entry = elf_load(task, buffer);
    struct thread* main = thread_creat(task, entry, 1);
    
    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    mmu_map(task->vm_map, stack - 0x1000, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    main->regs.rsp = stack;
    main->regs.rbp = stack;

    list_push_back(&task->threads, main);

    list_push_back(&task->fds, vfs_open(vfs_resolve("/dev/tty")));
    list_push_back(&task->fds, vfs_open(vfs_resolve("/dev/tty")));

    return task;
}

struct task* task_kcreat(struct task* parent, uintptr_t entry)
{
    struct task* task = mktask(parent);

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

    task->id = s_id++;
    task->vm_map = mmu_clone_vmmap(src->vm_map);
    task->threads = list_create();
    task->fds = list_create();
    task->children = list_create();
    task->parent = src;

    struct thread* main = thread_clone(task, calling);
    list_push_back(&task->threads, main);

    LIST_FOREACH(&src->fds)
    {
        struct fd* fd = node->data;
        list_push_back(&task->fds, vfs_open(fd->filp));
    }

    return task;
}

void task_execve(struct task* task, const char* path, const char* argv[], const char* envp[])
{
    struct task* parent = task->parent;
    int id = task->id;
    task_destroy(task);

    struct file* file = vfs_resolve(path);
    void* data = kmalloc(file->size);
    vfs_read(file, data, 0, file->size);

    struct task* new = task_creat(parent, data, argv, envp);
    memcpy(task, new, sizeof(struct task));
    sched_start(task);
}

void task_destroy(struct task* task)
{
    // TODO: implement properly
    LIST_FOREACH(&task->threads)
    {
        struct thread* thread = node->data;
        thread->state = THREAD_DEAD;
    }
}

struct task* task_curr()
{
    return thread_curr()->parent;
}