#include <sys.h>
#include <types.h>
#include <task.h>
#include <vfs.h>
#include <reg.h>
#include <cpu.h>
#include <thread.h>

// TODO: API folder

static int sys_open(const char* path, uint32_t flags, mode_t mode)
{
    struct task* task = task_curr();
    list_push_back(&task->fds, vfs_open(vfs_resolve(path))); // TODO: canonicalize the path first
    return task->fds.size - 1;
}

static int sys_close(int fd)
{
    // TODO: remove list element at
}

static ssize_t sys_read(int fdno, void* buf, size_t size)
{
    struct task* task = task_curr();
    //if (fd < 0 || fd >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);
    ssize_t ret = vfs_read(fd->filp, buf, fd->off, size);
    fd->off += size;
    return ret;
}

static ssize_t sys_write(int fdno, const void* buf, size_t size)
{
    struct task* task = task_curr();
    //if (fd < 0 || fd >= task->fds.size) return -EBADF;

    struct fd* fd = list_get(&task->fds, fdno);
    ssize_t ret = vfs_write(fd->filp, buf, fd->off, size);
    fd->off += size;
    return ret;
}

static int sys_fork()
{
    struct thread* calling = thread_curr();

    //calling->regs = calling->syscall_regs; // Take this oppurtunity to save the new registers
    struct task* child = task_clone(task_curr(), calling);

    memcpy(&((struct thread*)child->threads.head->data)->regs, &calling->syscall_regs, sizeof(struct regs));
    ((struct thread*)child->threads.head->data)->regs.rax = 0; // return 0 to the child task

    printk("%x %x %x\n", ((struct thread*)child->threads.head->data)->regs.rip, child->threads.head->data, calling);

    sched_start(child);

    return child->id;
}

typedef uintptr_t (*syscall_t)();

static syscall_t syscalls[] =
{
    sys_open,
    sys_close,
    sys_read,
    sys_write,
    sys_fork
};

void syscall_handler(struct regs* r)
{
    thread_curr()->syscall_regs = *r;
    syscall_t sc = syscalls[arch_syscall_num(r)];
    arch_syscall_ret(r, sc(ARCH_SCARG0(r),
                           ARCH_SCARG1(r),
                           ARCH_SCARG2(r),
                           ARCH_SCARG3(r),
                           ARCH_SCARG4(r),
                           ARCH_SCARG5(r)));
}

void sys_init()
{
    idt_set_handler(128, syscall_handler);
}