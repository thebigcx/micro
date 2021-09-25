#include <micro/init.h>
#include <micro/debug.h>
#include <micro/vfs.h>
#include <micro/task.h>
#include <micro/sched.h>
#include <micro/fs.h>
#include <micro/heap.h>
#include <micro/stdlib.h>
#include <micro/fbdev.h>
#include <micro/sys.h>
#include <micro/errno.h>
#include <micro/ioctls.h>
#include <micro/termios.h>
#include <micro/tty.h>
#include <micro/module.h>
#include <micro/devfs.h>
#include <micro/dev.h>

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
    initrd->end   = end;

    file->ops.read  = initrd_read;
    file->ops.write = initrd_write;
    file->type      = FL_BLKDEV;
    file->device    = initrd;
    file->perms     = 0660;

    strcpy(file->name, "initrd");
    devfs_register(file);
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
            file->type = FL_FILE;

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

int initramfs_mount(const char* dev, const void* data, struct file* fsroot)
{
    struct file* device = kmalloc(sizeof(struct file));
    vfs_resolve(dev, device, 1);
    struct initramfs* ramfs = kmalloc(sizeof(struct initramfs));
    ramfs->device = device->device;

    fsroot->ops.find = initramfs_find;
    fsroot->type = FL_DIR;
    fsroot->device = ramfs;

    return 0;
}

void initramfs_init()
{
    vfs_register_fs("initramfs", initramfs_mount);
}

static void kmod_load(const char* path)
{
    struct file* mod = kmalloc(sizeof(struct file));
    vfs_resolve(path, mod, 1);
    void* buffer = kmalloc(mod->size);
    vfs_read(mod, buffer, 0, mod->size);

    module_load(buffer, mod->size);
}

void generic_init(struct genbootparams params)
{
    sys_init();

    printk("initializing VFS\n");
    vfs_init();

    devfs_init();

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

    kmod_load("/lib/modules/ps2kb.ko");

    init_devices();

    tty_init();

    //vga_init();
    fb_init_dev();

    printk("starting scheduler\n");
    sched_init();
}