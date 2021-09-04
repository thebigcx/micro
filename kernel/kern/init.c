#include <micro/init.h>
#include <micro/debug.h>
#include <micro/vfs.h>
#include <micro/task.h>
#include <micro/sched.h>
#include <micro/ps2.h>

struct fheader
{
    char name[128];
    uint64_t size;
};

struct initrd
{
    uintptr_t start, end;
};

struct initrd_file
{
    uintptr_t start;
};

ssize_t initrd_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct initrd_file* initrd_file = file->device;
    memcpy(buf, initrd_file->start + off, size);
    return size;
}

struct file* initrd_find(struct file* dir, const char* name)
{
    struct initrd* initrd = dir->device;
    struct fheader* curr = (struct fheader*)initrd->start;

    while ((uintptr_t)curr < initrd->end)
    {
        if (!strcmp(curr->name, name))
        {
            struct file* file = kmalloc(sizeof(struct file));
            file->ops.read = initrd_read;
            file->flags = FL_FILE;

            struct initrd_file* initrd_file = kmalloc(sizeof(struct initrd_file));
            initrd_file->start = (uintptr_t)curr + sizeof(struct fheader);

            file->size = curr->size;
            file->device = initrd_file;

            return file;
        }

        curr = (struct fheader*)((uintptr_t)curr + sizeof(struct fheader) + curr->size);
    }

    return NULL;
}

void initrd_init(uintptr_t start, uintptr_t end)
{
    struct initrd* initrd = kmalloc(sizeof(struct initrd));
    initrd->start = start;
    initrd->end = end;

    struct file* file = kmalloc(sizeof(struct file));
    file->ops.find = initrd_find;
    file->flags = FL_DIR;
    file->device = initrd;

    vfs_mount(file, "/initrd");
}

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
    ssize_t kbsize = 0;

    for (size_t i = 0; i < size; i++)
    {
        if (raw[i] < 88)
        {
            ((char*)buf)[kbsize++] = ascii[*raw];
        }

        buf++;
    }

    kfree(raw);

    return kbsize;
}

ssize_t tty_write(struct file* file, const void* buf, off_t off, size_t size)
{
    printk("%s", (char*)buf);
    return size;
}

void generic_init(struct genbootparams params)
{
    sys_init();

    printk("initializing VFS\n");
    vfs_init();
    
    ps2_init();

    printk("mounting initial ramdisk\n");
    initrd_init(params.initrd_start, params.initrd_end);

    //module_load("/initrd/test.ko");
    //for (;;);

    // TODO: temporary

    struct file* file = kmalloc(sizeof(struct file));
    file->ops.read = tty_read;
    file->ops.write = tty_write;
    file->flags = FL_CHARDEV;
    vfs_mount(file, "/dev/tty");

    sched_start(task_init_creat());

    printk("starting scheduler\n");
    sched_init();
}