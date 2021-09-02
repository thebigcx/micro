#include <micro/init.h>
#include <micro/debug.h>
#include <micro/vfs.h>
#include <micro/task.h>
#include <micro/sched.h>

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

ssize_t tty_read(struct file* file, void* buf, off_t off, size_t size)
{
    memset(buf, '6', size);
    return 0;
}

ssize_t tty_write(struct file* file, const void* buf, off_t off, size_t size)
{
    printk("%s", (char*)buf);
    return size;
}

void generic_init(struct genbootparams params)
{
    sys_init();

    vfs_init();

    initrd_init(params.initrd_start, params.initrd_end);

    // TODO: FIXME: TESTS

    struct file* file = kmalloc(sizeof(struct file));
    file->ops.read = tty_read;
    file->ops.write = tty_write;
    file->flags = FL_CHARDEV;
    vfs_mount(file, "/dev/tty");

    //char* relat;
    //struct file* tty = vfs_getmnt("/dev/tty", &relat);
    //ASSERT(file == tty);

    struct fd* fd = vfs_open(vfs_resolve("/dev/tty"));
    printk("%x %x\n", fd->filp, file);

    struct file* init = vfs_resolve("/initrd/init");
    printk("%x\n", init);

    void* buffer = kmalloc(init->size);
    vfs_read(init, buffer, 0, init->size);

    //void* buffer = initrd_read("init");
    struct task* init_task = task_creat(NULL, buffer, NULL, NULL);
    sched_start(init_task);

//    for(;;);

    printk("starting scheduler\n");
    sched_init();

    for (;;); 
}