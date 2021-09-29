#include <micro/task.h>
#include <micro/sched.h>
#include <micro/heap.h>
#include <arch/cpu.h>
#include <micro/thread.h>
#include <micro/debug.h>
#include <micro/binfmt.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <micro/errno.h>

// TODO: needs WAY more locking

static unsigned int s_id = 1;

static struct task* mktask(struct task* parent, struct vm_map* vm_map)
{
    struct task* task = kmalloc(sizeof(struct task));
    memset(task, 0, sizeof(struct task));

    task->threads  = list_create();
    task->fds      = kmalloc(sizeof(struct fd*) * FD_MAX);
    task->id       = s_id++;
    task->vm_map   = vm_map;
    task->children = list_create();
    task->parent   = parent;
    task->sigqueue = list_create();
    task->umask    = 0022;
    
    strcpy(task->workd, "/");
    memset(task->fds, 0, sizeof(struct fd*) * FD_MAX);

    if (parent)
        list_push_back(&parent->children, task);

    return task;
}

// TODO: fix this up - should move into a more syscall-oriented approach, as this
// is never called from the kernel (execept for /bin/init)
static void init_user_task(struct task* task, const char* path,
                          const char* argv[], const char* envp[], uintptr_t entry)
{
    task->main = thread_creat(task, 0, 1);

    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    struct vm_area* area = vm_map_anon(task->vm_map, stack - 0x4000, 0x4000, 0);

    // Allocate the first page
    vm_map_anon_alloc(task->vm_map, area, area->base, 4 * PAGE4K);

    task->main->regs.rsp = stack;
    task->main->regs.rbp = stack;
    task->main->regs.rip = entry;

    struct vm_area* sigstack = vm_map_anon(task->vm_map, 0, PAGE4K, 0);
    task->sigstack = sigstack + PAGE4K;

    setup_user_stack(task, argv, envp);

    list_push_back(&task->threads, task->main);
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

    struct file file;
    int e;
    if ((e = vfs_resolve("/init", &file, 1))) return e;

    void* data = kmalloc(file.size);
    vfs_read(&file, data, 0, file.size);

    const char* argv[] = { "/init", NULL };
    const char* envp[] = { NULL };

    uintptr_t entry;
    if ((e = elf_load(task->vm_map, data, argv, envp, &entry)))
        return e;

    init_user_task(task, argv[0], argv, envp, entry);

    struct file* null = kmalloc(sizeof(struct file));
    vfs_resolve("/dev/null", null, 1);
    task->fds[0] = vfs_open(null, 0, 0);
    task->fds[1] = vfs_open(null, 0, 0);
    task->fds[2] = vfs_open(null, 0, 0);

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

    task->euid = src->euid;
    task->egid = src->egid;
    task->ruid = src->ruid;
    task->rgid = src->rgid;

    if (src->groupcnt)
    {
        task->groupcnt = src->groupcnt;
        task->groups = kmalloc(task->groupcnt * sizeof(gid_t));

        for (size_t i = 0; i < src->groupcnt; i++)
            task->groups[i] = src->groups[i];
    }

    memcpy(task->signals, src->signals, sizeof(struct sigaction) * 32);

    return task;
}

// TODO: move into exec.c
int task_execve(struct task* task, const char* path, const char* argv[], const char* envp[])
{
    struct file file;
    int e;
    if ((e = vfs_resolve(path, &file, 1))) return e;

    void* data = kmalloc(file.size);
    vfs_read(&file, data, 0, file.size);

    const char* interp = elf_getinterp(data);
    if (interp)
    {
        size_t argc = 1;
        while (argv[argc - 1]) argc++;

        char* nargv[argc + 1];
        nargv[0] = interp;
        memcpy(&nargv[1], &argv[0], argc * sizeof(const char*));

        // TODO: this leaks memory
        struct file interp;

        int e;
        if ((e = vfs_resolve(nargv[0], &interp, 1))) return e;

        // Don't know why POSIX defines these as different error codes
        if (interp.type == FL_DIR) return -EISDIR;
        if (interp.type != FL_FILE) return -EACCES;

        return task_execve(task, nargv[0], nargv, envp);
    }

    struct vm_map* vm_map = mmu_create_vmmap();

    uintptr_t entry;
    if ((e = elf_load(vm_map, data, argv, envp, &entry)))
        return e;

    // Only delete vm_map and set new one if elf_load passed

    // About to delete the in-use pml4
    mmu_set_kpml4();

    mmu_destroy_vmmap(task->vm_map);
    task->vm_map = vm_map;
    
    lcr3(task->vm_map->pml4_phys);

    // Clean threads
    LIST_FOREACH(&task->threads)
    {
        struct thread* thread = node->data;
        thread->state = THREAD_DEAD;
    }
    list_clear(&task->threads);
    
    init_user_task(task, path, argv, envp, entry);

    sched_start(task); // Start the new main thread
    return 0;
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

    task->status = status;
    task->state = TASK_DEAD;
    task->changed = 1;

    if (task->waiter)
    {
        sched_spawnthread(task->waiter);
        task->waiter = NULL;
    }

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
    list_enqueue(&task->sigqueue, sig);
}

struct task* task_curr()
{
    if (!thread_curr()) return NULL;
    return thread_curr()->parent;
}