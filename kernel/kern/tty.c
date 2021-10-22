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
#include <micro/sched.h>
#include <micro/debug.h>
#include <micro/lock.h>

// TODO: use a wakeup queue
static ssize_t iobuf_read(struct ringbuf* rbuf, void* buf, size_t size)
{
    ssize_t bytes;
    while (!(bytes = ringbuf_size(rbuf))) sched_yield();

    if (size < (size_t)bytes) bytes = size;
    
    ringbuf_read(rbuf, buf, bytes);
    return bytes;
}

ssize_t pts_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pt* pt = file->inode->priv;
    return iobuf_read(pt->inbuf, buf, size);
/*
    // TODO: use a wakeup queue instead (like a semaphore)
    ssize_t bytes;
    while (!(bytes = ringbuf_size(pt->inbuf))) sched_yield();

    if (size < (size_t)bytes) bytes = size;

    ringbuf_read(pt->inbuf, buf, bytes);

    return bytes;*/
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
        case TIOCGPGRP:
        {
            return pt->fgid;
        }
        case TIOCSPGRP:
        {
            pt->fgid = *(pid_t*)argp;
            return 0;
        }
        case FIONREAD:
        {
            *(int*)argp = 0;
            return 0;
        }
    }

    printk("warning: ioctl request=%d of TTY not implemented!\n", req);
    return -EINVAL;
}

// TODO: use iobuf_read(pt->outbuf, buf, size);
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

    if (pt->termios.c_iflag & ECHO)
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
    size_t i = 0, bytes = 0;
    LIST_FOREACH(&slaves)
    {
        if (bytes + sizeof(struct dirent) > size) break;
        if (off--) continue;

        struct dentry* dev = node->data;
        
        strcpy(dirp[i].d_name, dev->name);
        dirp[i].d_type   = IFTODT(dev->file->mode & S_IFMT);
        dirp[i].d_reclen = sizeof(struct dirent);
        dirp[i].d_off    = sizeof(struct dirent) * (i + 1);
       
        i++; 
        bytes += sizeof(struct dirent);
    }

    return i;
}

struct inode* ptm_open(struct pt* pt)
{
    struct inode* ptm = kcalloc(sizeof(struct inode));

    ptm->fops.read    = ptm_read;
    ptm->fops.write   = ptm_write;
    ptm->fops.ioctl   = pt_ioctl;
    ptm->mode         = S_IFCHR | 0620;
    ptm->priv         = pt;

    return ptm;
}

static int s_ptsname = 0;
static lock_t s_ptsname_lock = 0;

struct inode* pts_open(struct pt* pt)
{
    struct inode* pts = kcalloc(sizeof(struct inode));

    pts->fops.read  = pts_read;
    pts->fops.write = pts_write;
    pts->fops.ioctl = pt_ioctl;
    pts->mode       = S_IFCHR | 0620;
    pts->priv       = pt;
    pts->uid        = task_curr()->euid;

    pts->ctime      = time_getepoch();
    pts->atime      = pts->ctime;
    pts->mtime      = pts->ctime;

    struct dentry* dentry = kmalloc(sizeof(struct dentry));

    // Generate the next name
    LOCK(s_ptsname_lock);
    
    pt->num = s_ptsname++;
    itoa(pt->num, dentry->name, 10);
    
    UNLOCK(s_ptsname_lock);
    
    dentry->file = pts;
    
    list_enqueue(&slaves, dentry);
    return pts;
}

static struct termios s_default_termios =
{
    .c_iflag = ECHO 
};

int ptmx_open_new(struct inode* inode, struct file* file)
{
    struct pt* pt = kcalloc(sizeof(struct pt));
    memset(pt, 0, sizeof(struct pt));

    pt->termios = s_default_termios;

    pt->inbuf  = ringbuf_create(1024);
    pt->outbuf = ringbuf_create(1024);
    pt->ptm    = ptm_open(pt);
    pt->pts    = pts_open(pt);

    file->ops = pt->ptm->fops;
    file->inode->priv = pt;

    return 0;
}

int ptsfs_mount(const char*, const void*, struct inode* fsroot)
{
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

    struct file_ops ptsops = { 0 };
    devfs_register(&ptsops, "pts", S_IFDIR | 0755, NULL);
}
