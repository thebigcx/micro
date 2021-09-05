#include <micro/task.h>
#include <micro/heap.h>
#include <arch/cpu.h>
#include <micro/thread.h>
#include <micro/debug.h>
#include <micro/binfmt.h>
#include <micro/vfs.h>

// TODO: needs WAY more locking

static unsigned int s_id = 1;

static struct task* mktask(struct task* parent, struct vm_map* vm_map)
{
    struct task* task = kmalloc(sizeof(struct task));

    task->threads = list_create();
    task->fds = list_create();
    task->id = s_id++;
    task->vm_map = vm_map;
    task->children = list_create();
    task->parent = parent;
    task->main = NULL;
    task->sigmask = 0;
    task->sigqueue = list_create();
    task->waiting = 0;
    
    strcpy(task->workd, "/");
    
    memset(task->signals, 0, sizeof(task->signals));

    if (parent)
        list_push_back(&parent->children, task);

    return task;
}

#define PUSH_STR(stack, x) { stack -= strlen(x) + 1; strcpy((char*)stack, x); }
#define PUSH(stack, type, x) { stack -= sizeof(type); *((type*)stack) = x; }

static void setup_user_stack(struct task* task, const char* path, const char* argv[], const char* envp[])
{
    int argc = 0;
    uintptr_t args[64];

    uintptr_t cr3 = rcr3();

    lcr3(task->vm_map->pml4_phys);

    // Push the raw strings onto the stack
    while (argv[argc])
    {
        PUSH_STR(task->main->regs.rsp, argv[argc]);
        args[argc] = task->main->regs.rsp;
        argc++;
    }
    
    argc++; // Name of program

    // Pointer-align the stack for char* argv[]
    task->main->regs.rsp -= (task->main->regs.rsp % 8);

    // Push pointers in reverse order
    for (int i = argc - 1; i >= 0; i--)
    {
        PUSH(task->main->regs.rsp, uintptr_t, args[i]);
    }

    lcr3(cr3);

    task->main->regs.rdi = argc;
    task->main->regs.rsi = task->main->regs.rsp;
}

static void init_user_task(struct task* task, const char* path, const char* argv[], const char* envp[])
{
    struct file* file = vfs_resolve(path);
    void* data = kmalloc(file->size);
    vfs_read(file, data, 0, file->size);

    uintptr_t entry = elf_load(task, data);
    task->main = thread_creat(task, entry, 1);
    
    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    mmu_map(task->vm_map, stack - 0x1000, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    task->main->regs.rsp = stack;
    task->main->regs.rbp = stack;

    setup_user_stack(task, path, argv, envp);

    list_push_back(&task->threads, task->main);

    list_push_back(&task->fds, vfs_open(vfs_resolve("/dev/tty")));
    list_push_back(&task->fds, vfs_open(vfs_resolve("/dev/tty")));
}

static void idle()
{
    for(;;);
}

struct task* task_idle()
{
    return task_kcreat(NULL, idle);
}

struct task* task_init_creat()
{
    struct task* task = mktask(NULL, mmu_create_vmmap());

    char* argv[] = { "/init", NULL };
    char* envp[] = { NULL };

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

struct task* task_clone(const struct task* src, struct thread* calling)
{
    struct task* task = mktask(src, mmu_clone_vmmap(src->vm_map));
    task->main = thread_clone(task, calling);

    list_push_back(&task->threads, task->main);

    LIST_FOREACH(&src->fds)
    {
        struct fd* fd = node->data;
        list_push_back(&task->fds, vfs_open(fd->filp));
    }

    strcpy(task->workd, src->workd);

    return task;
}

void task_execve(struct task* task, const char* path, const char* argv[], const char* envp[])
{
    mmu_destroy_vmmap(task->vm_map);
    task->vm_map = mmu_create_vmmap();

    // Clean threads
    LIST_FOREACH(&task->threads)
    {
        struct thread* thread = node->data;
        thread->state = THREAD_DEAD;
    }
    list_clear(&task->threads);

    // Clean fd's
    LIST_FOREACH(&task->fds)
    {
        vfs_close((struct fd*)node->data);
    }
    list_clear(&task->fds);
    
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

    LIST_FOREACH(&task->fds)
    {
        vfs_close((struct fd*)node->data);
    }
    list_clear(&task->fds);

    task->main = NULL;

    mmu_destroy_vmmap(task->vm_map);

    if (task->parent) task_send(task->parent, SIGCHLD);

    //kfree(task);

    switch_next();
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