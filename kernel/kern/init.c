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
#include <micro/errno.h>
#include <micro/ioctls.h>
#include <micro/termios.h>
#include <micro/tty.h>
#include <micro/module.h>

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
    struct file* file = vfs_create_file();
    struct initrd* initrd = kmalloc(sizeof(struct initrd));
    initrd->start = start;
    initrd->end = end;

    file->ops.read = initrd_read;
    file->ops.write = initrd_write;
    file->flags = FL_BLOCKDEV;
    file->device = initrd;

    vfs_addnode(file, "/dev/initrd");
}

struct fheader
{
    char name[128];
    uint64_t size;
};

struct initramfs_file
{
    uintptr_t start;
};

struct initramfs
{
    struct initrd* device;
};

ssize_t initramfs_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct initramfs_file* fs_file = file->device;
    memcpy(buf, (void*)(fs_file->start + off), size);
    return size;
}

struct file* initramfs_find(struct file* dir, const char* name)
{
    struct initrd* initrd = ((struct initramfs*)dir->device)->device;
    struct fheader* curr = (struct fheader*)initrd->start;

    while ((uintptr_t)curr < initrd->end)
    {
        if (!strcmp(curr->name, name))
        {
            struct file* file = kmalloc(sizeof(struct file));
            file->ops.read = initramfs_read;
            file->flags = FL_FILE;

            struct initramfs_file* ramfile = kmalloc(sizeof(struct initramfs_file));
            ramfile->start = (uintptr_t)curr + sizeof(struct fheader);

            file->size = curr->size;
            file->device = ramfile;

            return file;
        }

        curr = (struct fheader*)((uintptr_t)curr + sizeof(struct fheader) + curr->size);
    }

    return NULL;
}

struct file* initramfs_mount(const char* dev, void* data)
{
    struct file* device = kmalloc(sizeof(struct file));
    vfs_resolve(dev, device);
    struct initramfs* ramfs = kmalloc(sizeof(struct initramfs));
    ramfs->device = device->device;

    struct file* file = kmalloc(sizeof(struct file));
    file->ops.find = initramfs_find;
    file->flags = FL_DIR;
    file->device = ramfs;

    return file;
}

void initramfs_init()
{
    vfs_register_fs("initramfs", initramfs_mount);
}

static void kmod_load(const char* path)
{
    struct file* mod = kmalloc(sizeof(struct file));
    vfs_resolve(path, mod);
    void* buffer = kmalloc(mod->size);
    vfs_read(mod, buffer, 0, mod->size);

    module_load(buffer, mod->size);
}

void generic_init(struct genbootparams params)
{
    sys_init();

    printk("initializing VFS\n");
    vfs_init();
    
    ps2_init();

    initrd_init(params.initrd_start, params.initrd_end);

    modules_init();

    initramfs_init();

    printk("mounting initramfs\n");
    vfs_mount_fs("/dev/initrd", "/", "initramfs", NULL);

    kmod_load("/ahci.ko");
    /*kmod_load("/fat.ko");

    printk("mounting root filesystem\n");
    vfs_mount_fs("/dev/sda", "/", "fat", NULL);*/

    kmod_load("/ext2.ko");

    printk("mounting root filesystem\n");
    vfs_mount_fs("/dev/sda", "/", "ext2", NULL);

    tty_init();

    //vga_init();
    fb_init_dev();

    printk("starting scheduler\n");
    sched_init();
}