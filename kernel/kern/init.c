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

/*struct fheader
{
    char name[128];
    uint64_t size;
};*/

struct initrd
{
    uintptr_t start, end;
};

/*struct initramfs_file
{
    uintptr_t start;
};

struct initramfs
{
    struct initrd* device;
};*/

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

/*ssize_t initramfs_read(struct file* file, void* buf, off_t off, size_t size)
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
    struct file* device = vfs_resolve(dev);
    struct initramfs* ramfs = kmalloc(sizeof(struct initramfs));
    ramfs->device = device->device;
    printk("%x\n", device->device);

    struct file* file = kmalloc(sizeof(struct file));
    file->ops.find = initramfs_find;
    file->flags = FL_MNTPT;
    file->device = ramfs;

    return file;
}*/

static char ascii[] =
{
    'c', '~', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't',
    'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n', '~', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\'', '`',
    '~', '\\', 'z', 'x', 'c', 'v', 'b',
    'n', 'm', ',', '.', '/', '~', '*',
    '~', ' ', '~', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c'
};

// FIXME: this is terrible - but at least it works
ssize_t tty_read(struct file* file, void* buf, off_t off, size_t size)
{
    uint8_t* raw = kmalloc(size);
    ssize_t bytes = kb_read(file, raw, off, size);
    if (bytes <= 0) return bytes;

    ssize_t kbsize = 0;

    char* cbuf = buf;

    for (size_t i = 0; i < size && i < (size_t)bytes; i++)
    {
        if (raw[i] < 88)
        {
            cbuf[kbsize++] = ascii[*raw];
        }

        cbuf++;
    }

    kfree(raw);

    return kbsize;
}

ssize_t tty_write(struct file* file, const void* buf, off_t off, size_t size)
{
    const char* cbuf = buf;
    while (size && size--)
    {
        fb_putch(*cbuf++, 0xffffffff, 0x0);
    }
    
    return size;
}

void generic_init(struct genbootparams params)
{
    sys_init();

    printk("initializing VFS\n");
    vfs_init();
    
    ps2_init();

    printk("mounting initial ramdisk\n");

    //vfs_register_fs("initramfs", initramfs_mount);

    struct file* file = kmalloc(sizeof(struct file));
    struct initrd* initrd = kmalloc(sizeof(struct initrd));
    initrd->start = params.initrd_start;
    initrd->end = params.initrd_end;

    file->ops.read = initrd_read;
    file->ops.write = initrd_write;
    file->flags = FL_BLOCKDEV;
    file->device = initrd;

    vfs_addnode(file, "/dev/initrd");

    fat_init();

    vfs_mount_fs("/dev/initrd", "/", "fat", NULL);

    // TEMP
    struct file* assert = vfs_resolve("/usr/include/assert.h");
    void* buf = kmalloc(512);
    memset(buf, 'X', 512);
    vfs_write(assert, buf, 0, 512);

    char* buf2 = kmalloc(assert->size);
    vfs_read(assert, buf2, 0, assert->size);

    for (int i = 0; i < assert->size; i++) printk("%c", buf2[i]);

    for (;;);

    struct file* tty = kmalloc(sizeof(struct file));
    tty->ops.read = tty_read;
    tty->ops.write = tty_write;
    tty->flags = FL_CHARDEV;
    vfs_addnode(tty, "/dev/tty");

    sched_start(task_init_creat());

    printk("starting scheduler\n");
    sched_init();
}