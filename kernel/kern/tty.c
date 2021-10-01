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
    struct pt* pt = file->priv;

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
    struct pt* pt = file->priv;

    ringbuf_write(pt->outbuf, buf, size);

    return size;
}

int pts_ioctl(struct file* file, unsigned long req, void* argp)
{
    struct pt* pt = file->priv;

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
    struct pt* pt = file->priv;

    ssize_t bytes = ringbuf_size(pt->outbuf);

    if (bytes <= 0) return 0;
    if (size < (size_t)bytes) bytes = size;

    ringbuf_read(pt->outbuf, buf, bytes);

    return size;
}

ssize_t ptm_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pt* pt = file->priv;

    ringbuf_write(pt->inbuf, buf, size);

    return size;
}

static struct list slaves;

int ptsfs_lookup(struct file* dir, const char* name, struct dentry* dentry)
{
    LIST_FOREACH(&slaves)
    {
        struct dentry* dev = node->data;
        if (!strcmp(name, dev->name))
        {
            strcpy(dentry->name, name);
            dentry->file = memdup(dev->file, sizeof(struct file));

            return 0;
        }
    }

    return -ENOENT;
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
    ptm->priv      = pt;
    
    return ptm;
}

struct file* pts_open(struct pt* pt)
{
    struct file* pts = vfs_create_file();

    pts->ops.read  = pts_read;
    pts->ops.write = pts_write;
    pts->mode      = S_IFCHR | 0620;
    pts->priv      = pt;

    // TODO: generate a unique name
    
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

int ptsfs_mount(const char* dev, const void* data, struct file* fsroot)
{
    (void)dev; (void)data;

    fsroot->ops.lookup   = ptsfs_lookup;
    fsroot->ops.getdents = ptsfs_getdents;
    fsroot->mode         = S_IFDIR | 0755;

    return 0;
}

void tty_init()
{
    slaves = list_create();

    struct file_ops ops = { .open = ptmx_open };
    devfs_register_blkdev(&ops, "ptmx", 0666, NULL);

    // Pseudoterminal slave filesystem
    vfs_register_fs("ptsfs", ptsfs_mount);
    vfs_mount_fs("", "/dev/pts", "ptsfs", NULL);
}

// TODO: use the /proc filesystem instead of this syscall
SYSCALL_DEFINE(ptsname, int fdno, char* buf, size_t n)
{
    // TODO: this is temporary!!
    strcpy(buf, "/dev/pts/0");
    return 0;

    PTRVALID(buf);
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    // TODO: struct file should hold flags like MASTER_PTY, DEVICE, etc (for fcntl() calls)
    // THIS IS DANGEROUS - IT MIGHT NOT BE A MASTER PTY
    struct pt* pt = fd->filp->priv;

    //if (strlen("/dev/pts/") + strlen(pt->pts->name) >= n) return -ERANGE;

    strcpy(buf, "/dev/pts/");
    //strcpy(buf + strlen(buf), pt->pts->name);

    return 0;
}