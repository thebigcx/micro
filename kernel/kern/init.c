#include <init.h>
#include <debug/syslog.h>
#include <vfs.h>
#include <task.h>
#include <sched.h>

static uintptr_t initrd_start;
static uintptr_t initrd_end;

struct fheader
{
    char name[128];
    uint64_t size;
};

void* initrd_read(const char* file)
{
    struct fheader* curr = (struct fheader*)initrd_start;

    while ((uintptr_t)curr < initrd_end)
    {
        if (!strcmp(curr->name, file))
        {
            void* buffer = kmalloc(curr->size);
            memcpy(buffer, (uintptr_t)curr + sizeof(struct fheader), curr->size);
            return buffer;
        }

        curr = (struct fheader*)((uintptr_t)curr + sizeof(struct fheader) + curr->size);
    }

    return NULL;
}

void initrd_init(uintptr_t start, uintptr_t end)
{
    initrd_start = start;
    initrd_end = end;
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
    vfs_mount(file, "/dev/tty");

    //char* relat;
    //struct file* tty = vfs_getmnt("/dev/tty", &relat);
    //ASSERT(file == tty);

    struct fd* fd = vfs_open(vfs_resolve("/dev/tty"));
    printk("%x %x\n", fd->filp, file);

    void* buffer = initrd_read("init");
    struct task* init = task_creat(NULL, buffer, NULL, NULL);
    sched_start(init);

//    for(;;);

    printk("starting scheduler");
    sched_init();

    for (;;); 
}