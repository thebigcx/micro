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
    struct pt* pt = file->inode->priv;

    //ssize_t bytes = ringbuf_size(pt->inbuf);

    //if (bytes <= 0) return 0;
    // TODO: use a wakeup queue instead (like a semaphore)
    ssize_t bytes;
    while (!(bytes = ringbuf_size(pt->inbuf))) sched_yield();

    if (size < (size_t)bytes) bytes = size;

    ringbuf_read(pt->inbuf, buf, bytes);

    return bytes;
}

ssize_t pts_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pt* pt = file->inode->priv;

    ringbuf_write(pt->outbuf, buf, size);

    return size;
}

int pt_ioctl(struct file* file, int req, void* argp)
{
    struct pt* pt = file->inode->priv;

    switch (req)
    {
        case TIOCGWINSZ:
        {
            memcpy(argp, &pt->size, sizeof(struct winsize));
            return 0;
        }
        case TIOCSWINSZ:
        {
            memcpy(&pt->size, argp, sizeof(struct winsize));
            //task_send(task_curr(), SIGWINCH);
            return 0;
        }
        case TCGETS:
        {
            memcpy(argp, &pt->termios, sizeof(struct termios));
            return 0;
        }

        case TCSETS:
        case TCSETSF:
        case TCSETSW:
        {
            memcpy(&pt->termios, argp, sizeof(struct termios));
            return 0;
        }
        case TIOCGPTN:
        {
            *((unsigned int*)argp) = pt->num;
            return 0;
        }
        case TIOCSPTLCK:
        {
            return 0;
        }
    }

    printk("warning: ioctl request=%d of TTY not implemented!\n", req);
    return -EINVAL;
}

ssize_t ptm_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pt* pt = file->inode->priv;

    ssize_t bytes = ringbuf_size(pt->outbuf);

    if (bytes <= 0) return 0;
    if (size < (size_t)bytes) bytes = size;

    ringbuf_read(pt->outbuf, buf, bytes);

    return size;
}

ssize_t ptm_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pt* pt = file->inode->priv;

    ringbuf_write(pt->inbuf, buf, size);

    //if (pt->termios.c_iflag & ECHO)
        ringbuf_write(pt->outbuf, buf, size);

    return size;
}

static struct list slaves;

int ptsfs_lookup(struct inode* dir, const char* name, struct dentry* dentry)
{
    LIST_FOREACH(&slaves)
    {
        struct dentry* dev = node->data;
        if (!strcmp(name, dev->name))
        {
            strcpy(dentry->name, name);
            dentry->file = memdup(dev->file, sizeof(struct inode));

            return 0;
        }
    }

    return -ENOENT;
}

ssize_t ptsfs_getdents(struct inode* dir, off_t off, size_t size, struct dirent* dirp)
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

struct inode* ptm_open(struct pt* pt)
{
    struct inode* ptm = vfs_create_file();

    ptm->fops.read    = ptm_read;
    ptm->fops.write   = ptm_write;
    ptm->fops.ioctl   = pt_ioctl;
    ptm->mode         = S_IFCHR | 0620;
    ptm->priv         = pt;
    
    return ptm;
}

struct inode* pts_open(struct pt* pt)
{
    struct inode* pts = vfs_create_file();

    pts->fops.read  = pts_read;
    pts->fops.write = pts_write;
    pts->fops.ioctl = pt_ioctl;
    pts->mode       = S_IFCHR | 0620;
    pts->priv       = pt;

    // TODO: generate a unique name
    
    struct dentry* dentry = kmalloc(sizeof(struct dentry));

    strcpy(dentry->name, "0");
    dentry->file = pts;
    
    list_enqueue(&slaves, dentry);

    return pts;
}

int ptmx_open_new(struct inode* inode, struct file* file)
{
    struct pt* pt = kcalloc(sizeof(struct pt));
    memset(pt, 0, sizeof(struct pt));

    pt->inbuf  = ringbuf_create(1024);
    pt->outbuf = ringbuf_create(1024);
    pt->ptm    = ptm_open(pt);
    pt->pts    = pts_open(pt);
    pt->num    = 0; // TODO: allocate

    file->ops = pt->ptm->fops;
    file->inode->priv = pt;

    //file->inode->priv = pt;
    //memcpy(file, pt->ptm, sizeof(struct inode));

    return 0;
}

int ptsfs_mount(const char* dev, const void* data, struct inode* fsroot)
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

    struct file_ops ops = { .open = ptmx_open_new };
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

    struct file* fd = task_curr()->fds[fdno];

    // TODO: struct inode should hold flags like MASTER_PTY, DEVICE, etc (for fcntl() calls)
    // THIS IS DANGEROUS - IT MIGHT NOT BE A MASTER PTY
    struct pt* pt = fd->inode->priv;

    //if (strlen("/dev/pts/") + strlen(pt->pts->name) >= n) return -ERANGE;

    strcpy(buf, "/dev/pts/");
    //strcpy(buf + strlen(buf), pt->pts->name);

    return 0;
}
