#include <sys.h>
#include <types.h>
#include <task.h>
#include <vfs.h>
#include <reg.h>
#include <cpu.h>

// TODO: API folder

static int sys_open(const char* path, uint32_t flags, mode_t mode)
{
    struct task* task = task_curr();
    list_push_back(&task->fds, vfs_open(path));
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

typedef uintptr_t (*syscall_t)();

static syscall_t syscalls[] =
{
    sys_open,
    sys_close,
    sys_read,
    sys_write
};

void syscall_handler(struct regs* r)
{
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