#include <micro/task.h>
#include <micro/sched.h>
#include <micro/heap.h>
#include <arch/cpu.h>
#include <micro/thread.h>
#include <micro/debug.h>
#include <micro/binfmt.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>

// TODO: needs WAY more locking

static unsigned int s_id = 1;

static struct task* mktask(struct task* parent, struct vm_map* vm_map)
{
    struct task* task = kmalloc(sizeof(struct task));

    task->threads  = list_create();
    task->fds      = kmalloc(sizeof(struct fd*) * FD_MAX);
    task->id       = s_id++;
    task->vm_map   = vm_map;
    task->children = list_create();
    task->parent   = parent;
    task->main     = NULL;
    task->sigmask  = 0;
    task->sigqueue = list_create();
    task->waiting  = 0;
    task->dead     = 0;
    task->status   = 0;
    
    strcpy(task->workd, "/");
    
    memset(task->signals, 0, sizeof(task->signals));
    memset(task->fds, 0, sizeof(struct fd*) * FD_MAX);

    if (parent)
        list_push_back(&parent->children, task);

    return task;
}

// TODO: fix this up - should move into a more syscall-oriented approach, as this
// is never called from the kernel (execept for /bin/init)
static void init_user_task(struct task* task, const char* path, const char* argv[], const char* envp[])
{
    struct file* file = vfs_resolve(path);
    void* data = kmalloc(file->size);
    vfs_read(file, data, 0, file->size);

    task->main = thread_creat(task, 0, 1);

    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    mmu_map(task->vm_map, stack - 0x1000, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    task->main->regs.rsp = stack;
    task->main->regs.rbp = stack;

    task->main->regs.rip = elf_load(task, data, argv, envp);

    list_push_back(&task->threads, task->main);

    // TEMP: DEBUG
    task->fds[0] = vfs_open(vfs_resolve("/dev/tty"), 0, 0);
    task->fds[1] = vfs_open(vfs_resolve("/dev/tty"), 0, 0);
    task->fds[2] = vfs_open(vfs_resolve("/dev/tty"), 0, 0);
}

static void idle()
{
    for(;;);
}

struct task* task_idle()
{
    return task_kcreat(NULL, (uintptr_t)idle);
}

struct task* task_init_creat()
{
    struct task* task = mktask(NULL, mmu_create_vmmap());

    const char* argv[] = { "/init", NULL };
    const char* envp[] = { NULL };

    init_user_task(task, argv[0], argv, envp);

    return task;
}

struct task* task_kcreat(struct task* parent, uintptr_t entry)
{
    struct task* task = mktask(parent, mmu_create_vmmap());

    task->main = thread_creat(task, entry, 0);

    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    mmu_map(task->vm_map, stack - 0x1000, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    task->main->regs.rsp = stack;
    task->main->regs.rbp = stack;

    list_push_back(&task->threads, task->main);

    return task;
}

struct task* task_clone(struct task* src, struct thread* calling)
{
    struct task* task = mktask(src, mmu_clone_vmmap(src->vm_map));
    task->main = thread_clone(task, calling);

    list_push_back(&task->threads, task->main);

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (src->fds[i])
        {
            task->fds[i] = vfs_open(src->fds[i]->filp, 0, 0);
        }
    }

    strcpy(task->workd, src->workd);

    return task;
}

void task_execve(struct task* task, const char* path, const char* argv[], const char* envp[])
{
    // About to delete the in-use pml4
    mmu_set_kpml4();
    mmu_destroy_vmmap(task->vm_map);
    task->vm_map = mmu_create_vmmap();
    lcr3(task->vm_map->pml4_phys);

    // Clean threads
    LIST_FOREACH(&task->threads)
    {
        struct thread* thread = node->data;
        thread->state = THREAD_DEAD;
    }
    list_clear(&task->threads);

    // Clean fd's
    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (task->fds[i])
        {
            vfs_close(task->fds[i]);
            task->fds[i] = NULL;
        }
    }
    
    init_user_task(task, path, argv, envp);

    sched_start(task); // Start the new main thread
}

void task_exit(int status)
{
    struct task* task = task_curr();

    LIST_FOREACH(&task->threads)
    {
        struct thread* thread = node->data;
        thread->state = THREAD_DEAD;
    }

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (task->fds[i])
        {
            vfs_close(task->fds[i]);
            task->fds[i] = NULL;
        }
    }

    task->main = NULL;

    if (task->parent) task_send(task->parent, SIGCHLD);
    task->dead = 1;
    task->status = status;

    switch_next();
}

void task_delete(struct task* task)
{
    mmu_destroy_vmmap(task->vm_map);
    kfree(task);
}

void task_send(struct task* task, int signal)
{
    int* sig = kmalloc(sizeof(int));
    *sig = signal;
    list_push_back(&task->sigqueue, sig);
}

struct task* task_curr()
{
    return thread_curr()->parent;
}