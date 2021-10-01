#include <micro/tty.h>
#include <micro/vfs.h>
#include <micro/ioctls.h>
#include <micro/termios.h>
#include <micro/errno.h>
#include <micro/heap.h>
#include <micro/fbdev.h>
#include <micro/ringbuf.h>
#include <micro/sys.h>
#include <micro/stdlib.h>
#include <micro/devfs.h>

ssize_t pts_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    //ssize_t bytes = ringbuf_size(pt->inbuf);

    //if (bytes <= 0) return 0;
    // TODO: use a wakeup queue instead (like a semaphore)
    ssize_t bytes;
    while (!(bytes = ringbuf_size(pt->inbuf))) sched_yield();

    if (size < (size_t)bytes) bytes = size;

    ringbuf_read(pt->inbuf, buf, bytes);

    return size;
}

ssize_t pts_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    ringbuf_write(pt->outbuf, buf, size);

    return size;
}

int pts_ioctl(struct file* file, unsigned long req, void* argp)
{
    struct pt* pt = file->device;

    switch (req)
    {
        case TIOCGWINSZ:
        {
            memcpy(argp, &pt->size, sizeof(struct winsize));
            return 0;
        }
        case TIOCSWINSZ:
        {
            memcpy(&pt->size, argp, sizeof(struct winsize)); // TODO: generate SIGWINCH
            return 0;
        }
    }

    return -EINVAL;
}

ssize_t ptm_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    ssize_t bytes = ringbuf_size(pt->outbuf);

    if (bytes <= 0) return 0;
    if (size < (size_t)bytes) bytes = size;

    ringbuf_read(pt->outbuf, buf, bytes);

    return size;
}

ssize_t ptm_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    ringbuf_write(pt->inbuf, buf, size);

    return size;
}

static struct list  slaves;
static struct file* ptsfs;

struct file* ptsfs_find(struct file* dir, const char* name)
{
    LIST_FOREACH(&slaves)
    {
        struct dentry* dev = node->data;
        if (!strcmp(name, dev->name))
        {
            struct file* copy = kmalloc(sizeof(struct file));
            memcpy(copy, dev->file, sizeof(struct file));
            return copy;
        }
    }

    return NULL;
}

ssize_t ptsfs_getdents(struct file* dir, off_t off, size_t size, struct dirent* dirp)
{
    size_t i;
    for (i = 0; i < size; i++)
    {
        if (i + off == slaves.size) break;

        struct dentry* dev = list_get(&slaves, i + off);
        strcpy(dirp[i].d_name, dev->name);
    }

    return i;
}

struct file* ptm_open(struct pt* pt)
{
    struct file* ptm = vfs_create_file();

    ptm->ops.read    = ptm_read;
    ptm->ops.write   = ptm_write;
    ptm->mode        = S_IFCHR | 0620;
    ptm->device      = pt;
    
    return ptm;
}

struct file* pts_open(struct pt* pt)
{
    struct file* pts = vfs_create_file();

    pts->ops.read    = pts_read;
    pts->ops.write   = pts_write;
    pts->mode        = S_IFCHR | 0620;
    pts->device      = pt;

    // TODO: generate a unique name
    //strcpy(pts->name, "0");
    
    struct dentry* dentry = kmalloc(sizeof(struct dentry));

    strcpy(dentry->name, "0");
    dentry->file = pts;
    
    list_enqueue(&slaves, dentry);

    return pts;
}

struct fd* ptmx_open(struct file* file, uint32_t flags, mode_t mode)
{
    struct pt* pt = kmalloc(sizeof(struct pt));
    memset(pt, 0, sizeof(struct pt));

    pt->inbuf  = ringbuf_create(1024);
    pt->outbuf = ringbuf_create(1024);
    pt->ptm    = ptm_open(pt);
    pt->pts    = pts_open(pt);

    return vfs_open(pt->ptm, 0, 0);
}

void tty_init()
{
    slaves = list_create();

    struct file* ptmx = vfs_create_file();

    ptmx->ops.open    = ptmx_open;
    ptmx->mode        = S_IFCHR | S_IFCHR;

    //strcpy(ptmx->name, "ptmx");
    devfs_register(ptmx, "ptmx");

    ptsfs = vfs_create_file();

    ptsfs->mode         = S_IFDIR | 0755;
    ptsfs->ops.find     = ptsfs_find;
    ptsfs->ops.getdents = ptsfs_getdents;

    //strcpy(ptsfs->name, "pts");
    devfs_register(ptsfs, "pts");
}

// TODO: use the /proc filesystem instead of this syscall
SYSCALL_DEFINE(ptsname, int fdno, char* buf, size_t n)
{
    strcpy(buf, "/dev/pts/0");
    return 0;

    PTRVALID(buf);
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    // TODO: struct file should hold flags like MASTER_PTY, DEVICE, etc (for fcntl() calls)
    // THIS IS DANGEROUS - IT MIGHT NOT BE A MASTER PTY
    struct pt* pt = fd->filp->device;

    //if (strlen("/dev/pts/") + strlen(pt->pts->name) >= n) return -ERANGE;

    strcpy(buf, "/dev/pts/");
    //strcpy(buf + strlen(buf), pt->pts->name);

    return 0;
}