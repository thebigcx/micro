#include <micro/task.h>
#include <micro/sched.h>
#include <micro/heap.h>
#include <arch/cpu.h>
#include <micro/thread.h>
#include <micro/debug.h>
#include <micro/binfmt.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/stdlib.h>
#include <micro/errno.h>
#include <micro/try.h>
#include <micro/vmmap.h>

// TODO: needs WAY more locking

static unsigned int s_pid = 1;

static struct task* mktask(struct task* parent, struct vm_map* vm_map)
{
    struct task* task = kcalloc(sizeof(struct task));

    task->threads  = list_create();
    task->fds      = kmalloc(sizeof(struct file*) * FD_MAX);
    task->pid      = s_pid++;
    task->vm_map   = vm_map;
    task->children = list_create();
    task->parent   = parent;
    task->umask    = 0022;
    
    strcpy(task->workd, "/");
    memset(task->fds, 0, sizeof(struct file*) * FD_MAX);

    if (parent)
        list_enqueue(&parent->children, task);

    return task;
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
    struct task* task = mktask(NULL, alloc_vmmap());
    
    char* const argv[] = { "/initrd/init", NULL };
    char* const envp[] = { NULL };
    
    do_exec(task, argv[0], argv, envp);
    return task;
}

struct task* task_kcreat(struct task* parent, uintptr_t entry)
{
    struct task* task = mktask(parent, alloc_vmmap());

    task->main = thread_creat(task, entry, 0);

    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    mmu_map(task->vm_map->pagemap, stack - 0x1000, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    task->main->regs.rsp = stack;
    task->main->regs.rbp = stack;

    list_enqueue(&task->threads, task->main);

    return task;
}

struct task* task_clone(struct task* src, struct thread* calling)
{
    struct task* task = mktask(src, fork_vmmap(src->vm_map));
    task->main = thread_clone(task, calling);

    list_enqueue(&task->threads, task->main);

    for (unsigned int i = 0; i < FD_MAX; i++)
    {
        if (src->fds[i])
        {
            task->fds[i] = memdup(src->fds[i], sizeof(struct file));
        }
    }

    strcpy(task->workd, src->workd);

    task->euid = src->euid;
    task->egid = src->egid;
    task->ruid = src->ruid;
    task->rgid = src->rgid;

    task->pgid = src->pgid;
    task->sid  = src->sid;

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
/*
// TODO: move into exec.c
int task_execve(struct task* task, const char* path, char* const argv[], char* const envp[])
{
    struct file file;
    TRY(vfs_open(path, &file, O_RDONLY));

    //CHECK_XPERM(file.inode->mode);

    if (!S_ISREG(file.inode->mode))
        return -ENOEXEC;

    void* data = kmalloc(file.inode->size);
    vfs_read(&file, data, file.inode->size);

    const char* interp = elf_getinterp(data);
    if (interp)
    {
        size_t argc = 1;
        while (argv[argc - 1]) argc++;

        char* nargv[argc + 1];
        nargv[0] = strdup(interp);
        memcpy(&nargv[1], &argv[0], argc * sizeof(const char*));

        struct file interp;
        TRY(vfs_open(nargv[0], &interp, O_RDONLY));

        if (S_ISDIR(interp.inode->mode)) return -EISDIR;
        if (!S_ISREG(interp.inode->mode)) return -EACCES;

        return task_execve(task, nargv[0], nargv, envp);
    }

    struct vm_map* vm_map = alloc_vmmap();

    struct elfinf inf;
    TRY(elf_load(vm_map, data, argv, envp, &inf))

    // Only delete vm_map and set new one if elf_load passed

    // About to delete the in-use pml4
    mmu_set_kpml4();

    free_vmmap(task->vm_map);
    task->vm_map = vm_map;
    
    lcr3(task->vm_map->pml4_phys);

    // Clean threads
    LIST_FOREACH(&task->threads)
    {
        struct thread* thread = node->data;
        thread->state = THREAD_DEAD;
    }
    list_clear(&task->threads);
    
    init_user_task(task, path, argv, envp, inf.entry, inf.brk);

    sched_start(task); // Start the new main thread
    return 0;
}
*/
void task_exit(int status)
{
    printk("task pid=%d exited\n", task_curr()->pid);
    struct task* task = task_curr();

    LIST_FOREACH(&task->threads)
    {
        struct thread* thread = node->data;
        thread->state = THREAD_DEAD;
    }

    struct task* init = sched_task_fromid(1);

    LIST_FOREACH(&task->children)
    {
        struct task* child = node->data;
        list_enqueue(&init->children, child);
    }
    list_clear(&task->children);

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

    task_change(task, TASK_DEAD);
    task->status = status;

    switch_next();
}

void task_delete(struct task* task)
{
    // Remove the child from the parent
    if (task->parent)
    {
        size_t i = 0;
        LIST_FOREACH(&task->parent->children)
        {
            if (node->data == task)
            {
                list_remove(&task->parent->children, i);
                break;
            }
            i++;
        }
    }

    free_vmmap(task->vm_map);
    kfree(task);
}

void task_send(struct task* task, int signal)
{
    struct signal* sig = kcalloc(sizeof(struct signal));
    sig->num = signal;
    sig->thr = NULL;
    list_enqueue(&task->main->sigqueue, sig);

    if (task == task_curr())
        sched_yield();
}

struct task* task_curr()
{
    if (!thread_curr()) return NULL;
    return thread_curr()->parent;
}

void task_change(struct task* task, int state)
{
    struct task* parent = task->parent;
    task->state = state;
    
    if (parent->waiting == task->pid
     || parent->waiting == -1)
    {
        thread_unblock(parent->main);
        parent->waiting = 0;
    }
}
