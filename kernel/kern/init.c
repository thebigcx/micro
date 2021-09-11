#include <micro/init.h>
#include <micro/debug.h>
#include <micro/vfs.h>
#include <micro/task.h>
#include <micro/sched.h>
#include <micro/fs.h>
#include <micro/heap.h>
#include <micro/stdlib.h>
#include <micro/ps2.h>
#include <micro/fb.h>
#include <micro/sys.h>
#include <micro/fat.h>
#include <micro/ext2.h>
#include <micro/errno.h>
#include <micro/ioctls.h>
#include <micro/termios.h>
#include <micro/tty.h>

struct initrd
{
    uintptr_t start, end;
};

ssize_t initrd_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct initrd* initrd = file->device;
    memcpy(buf, (void*)(initrd->start + off), size);
    return size;
}

ssize_t initrd_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct initrd* initrd = file->device;
    memcpy((void*)(initrd->start + off), buf, size);
    return size;
}

void initrd_init(uintptr_t start, uintptr_t end)
{
    printk("mounting initial ramdisk\n");
    struct file* file = kmalloc(sizeof(struct file));
    struct initrd* initrd = kmalloc(sizeof(struct initrd));
    initrd->start = start;
    initrd->end = end;

    file->ops.read = initrd_read;
    file->ops.write = initrd_write;
    file->flags = FL_BLOCKDEV;
    file->device = initrd;

    vfs_addnode(file, "/dev/initrd");
}

void generic_init(struct genbootparams params)
{
    sys_init();

    printk("initializing VFS\n");
    vfs_init();
    
    ps2_init();

    initrd_init(params.initrd_start, params.initrd_end);

    fat_init();

    vfs_mount_fs("/dev/initrd", "/", "fat", NULL);

    tty_init();

    printk("starting scheduler\n");
    sched_init();
}