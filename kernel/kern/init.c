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
#include <micro/procfs.h>
#include <micro/dev.h>
#include <micro/fcntl.h>

struct initrd
{
    uintptr_t start, end;
};

ssize_t initrd_read(struct fd* file, void* buf, off_t off, size_t size)
{
    struct initrd* initrd = file->filp->priv;
    memcpy(buf, (void*)(initrd->start + off), size);
    return size;
}

ssize_t initrd_write(struct fd* file, const void* buf, off_t off, size_t size)
{
    struct initrd* initrd = file->filp->priv;
    memcpy((void*)(initrd->start + off), buf, size);
    return size;
}

void initrd_init(uintptr_t start, uintptr_t end)
{
    printk("mounting initial ramdisk\n");

    struct initrd* initrd = kmalloc(sizeof(struct initrd));

    initrd->start = start;
    initrd->end   = end;

    struct new_file_ops ops = { .read = initrd_read, .write = initrd_write };
    devfs_register_blkdev(&ops, "initrd", 0660, initrd);
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

ssize_t initramfs_read(struct fd* file, void* buf, off_t off, size_t size)
{
    struct initramfs_file* fs_file = file->filp->priv;
    memcpy(buf, (void*)(fs_file->start + off), size);
    return size;
}

struct file* initramfs_find(struct file* dir, const char* name)
{   
    struct initrd* initrd = ((struct initramfs*)dir->priv)->device;
    struct fheader* curr = (struct fheader*)initrd->start;

    while ((uintptr_t)curr < initrd->end)
    {
        if (!strcmp(curr->name, name))
        {
            struct file* file = kmalloc(sizeof(struct file));
            //file->ops.read = initramfs_read;
            file->fops.read = initramfs_read;
            file->mode = S_IFREG | 0555;

            struct initramfs_file* ramfile = kmalloc(sizeof(struct initramfs_file));
            ramfile->start = (uintptr_t)curr + sizeof(struct fheader);

            file->size = curr->size;
            file->priv = ramfile;

            return file;
        }

        curr = (struct fheader*)((uintptr_t)curr + sizeof(struct fheader) + curr->size);
    }

    return NULL;
}

int initramfs_lookup(struct file* dir, const char* name, struct dentry* dentry)
{
    struct initrd* initrd = ((struct initramfs*)dir->priv)->device;
    struct fheader* curr = (struct fheader*)initrd->start;

    while ((uintptr_t)curr < initrd->end)
    {
        if (!strcmp(curr->name, name))
        {
            dentry->file = kmalloc(sizeof(struct file));
            dentry->file->fops.read = initramfs_read;
            dentry->file->mode = S_IFREG | 0555;

            struct initramfs_file* ramfile = kmalloc(sizeof(struct initramfs_file));
            ramfile->start = (uintptr_t)curr + sizeof(struct fheader);

            dentry->file->size = curr->size;
            dentry->file->priv = ramfile;

            strcpy(dentry->name, name);

            return 0;
        }

        curr = (struct fheader*)((uintptr_t)curr + sizeof(struct fheader) + curr->size);
    }

    return -ENOENT;
}

int initramfs_mount(const char* dev, const void* data, struct file* fsroot)
{
    struct file* device = kmalloc(sizeof(struct file));
    vfs_resolve(dev, device, 1);
    struct initramfs* ramfs = kmalloc(sizeof(struct initramfs));
    ramfs->device = device->priv;

    fsroot->ops.lookup = initramfs_lookup;
    fsroot->mode     = S_IFDIR | 0755;
    fsroot->priv     = ramfs;

    return 0;
}

void initramfs_init()
{
    vfs_register_fs("initramfs", initramfs_mount);
}

static void kmod_load(const char* path)
{
    //struct file* mod = kmalloc(sizeof(struct file));
    //vfs_resolve(path, mod, 1);
    //void* buffer = kmalloc(mod->size);
    //vfs_read(mod, buffer, 0, mod->size);

    struct fd mod;
    vfs_open_new(path, &mod, O_RDONLY);

    void* buffer = kmalloc(mod.filp->size);
    vfs_read_new(&mod, buffer, mod.filp->size);

    module_load(buffer, mod.filp->size);
}

void generic_init(struct genbootparams params)
{
    sys_init();

    printk("initializing VFS\n");
    vfs_init();

    devfs_init();
    procfs_init();

    initrd_init(params.initrd_start, params.initrd_end);

    modules_init();

    initramfs_init();

    printk("mounting initramfs\n");
    vfs_mount_fs("/dev/initrd", "/", "initramfs", NULL);

    printk("loading init modules\n");
    kmod_load("/ahci.ko");
    kmod_load("/ext2.ko");
    printk("mounting root filesystem\n");

    vfs_umount_fs("/"); // Unmount old initramfs
    vfs_mount_fs("/dev/sda", "/", "ext2", NULL);

    kmod_load("/lib/modules/ps2kb.ko");

    init_devices();

    tty_init();

    fb_init_dev();

    printk("starting scheduler\n");
    sched_init();
}