#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/heap.h>
#include <micro/sched.h>
#include <micro/stdlib.h>
#include <micro/try.h>
#include <micro/debug.h>
#include <micro/binfmt.h>
#include <arch/mmu.h>
#include <micro/thread.h>

int do_exec(struct task* task, const char* path, char* const argv[], char* const envp[])
{
    struct file elf;
    TRY(vfs_open(path, &elf, O_RDONLY));

    if (!S_ISREG(elf.inode->mode)) return -ENOEXEC;

    void* data = kmalloc(elf.inode->size);
    vfs_read(&elf, data, elf.inode->size);

    vfs_close(&elf);

    // Make sure it is a valid executable
    TRY(elf_valid(data));

    // From here on, no errors can occur
    mmu_set_kpml4();
    //vm_map_clear(task->vm_map);
    // TODO: clear mappings, rather than destroy the ENTIRE map
    free_vmmap(task->vm_map);
    task->vm_map = alloc_vmmap();

    // Top of canonical lower-half
    uintptr_t stack = 0x8000000000;
    size_t size = 0x16000; // TODO: dynamic stack size (can expand if necessary)
    struct vm_area* area = vm_map_anon(task->vm_map, stack - size, size, 0);

    // Allocate the first page for stack setup (args, environment, auxiliary)
    vm_map_anon_alloc(task->vm_map, area, area->end - 0x1000, 0x1000);

    struct elfinf info;
    elf_load(task->vm_map, data, argv, envp, &info);

    LIST_FOREACH(&task->threads)
        thread_free(node->data);
    list_clear(&task->threads);

    task->main = thread_creat(task, 0, 1);
    list_enqueue(&task->threads, task->main);

    task->main->regs.rsp = stack;
    task->main->regs.rbp = stack;
    task->main->regs.rip = info.entry;
    task->brk = info.brk;

    setup_user_stack(task, argv, envp);
   
    return 0;
}

SYSCALL_DEFINE(execve, const char* path, char* const argv[], char* const envp[])
{
    printk("execve(%s) of pid=%d\n", path, task_curr()->pid);
    PTRVALID(path);
    PTRVALID(argv);
    PTRVALID(envp);

    // Copy the arguments and environment variables from userspace
    char* arg_copy[16], *env_copy[16];
    size_t argc, envc;
    
    for (argc = 0; argv[argc]; argc++)
        arg_copy[argc] = strdup(argv[argc]);
    for (envc = 0; envp[envc]; envc++)
        env_copy[envc] = strdup(envp[envc]);

    arg_copy[argc] = NULL;
    env_copy[envc] = NULL;

    // Canonicalize the executable path
    char canon[256];
    vfs_mkcanon(path, task_curr()->workd, canon);

    TRY(do_exec(task_curr(), canon, arg_copy, env_copy));
    sched_start(task_curr());
    
    sched_yield();
    __builtin_unreachable();
}
