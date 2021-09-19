#include <micro/tty.h>
#include <micro/vfs.h>
#include <micro/ioctls.h>
#include <micro/termios.h>
#include <micro/errno.h>
#include <micro/heap.h>
#include <micro/ps2.h>
#include <micro/fbdev.h>
#include <micro/ringbuf.h>
#include <micro/sys.h>
#include <micro/stdlib.h>
#include <micro/devfs.h>

ssize_t pts_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    ssize_t bytes = ringbuf_size(pt->inbuf);

    if (bytes <= 0) return 0;
    if (size < bytes) bytes = size;

    ringbuf_read(pt->inbuf, buf, bytes);

    return size;
}

ssize_t pts_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    ringbuf_write(pt->outbuf, buf, size);

    return size;
}

ssize_t ptm_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    ssize_t bytes = ringbuf_size(pt->outbuf);

    if (bytes <= 0) return 0;
    if (size < bytes) bytes = size;

    ringbuf_read(pt->outbuf, buf, bytes);

    return size;
}

ssize_t ptm_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pt* pt = file->device;

    ringbuf_write(pt->inbuf, buf, size);

    return size;
}

struct file* ptm_open(struct pt* pt)
{
    struct file* ptm = vfs_create_file();

    ptm->ops.read    = ptm_read;
    ptm->ops.write   = ptm_write;
    ptm->flags       = FL_CHARDEV;
    ptm->device      = pt;
    
    return ptm;
}

struct file* pts_open(struct pt* pt)
{
    struct file* pts = vfs_create_file();

    pts->ops.read    = pts_read;
    pts->ops.write   = pts_write;
    pts->flags       = FL_CHARDEV;
    pts->device      = pt;

    // TODO: generate a unique name
    vfs_addnode(pts, "/dev/pts/0");

    return pts;
}

struct fd* ptmx_open(struct file* file, uint32_t flags, mode_t mode)
{
    struct pt* pt = kmalloc(sizeof(struct pt));

    pt->inbuf  = ringbuf_create(1024);
    pt->outbuf = ringbuf_create(1024);
    pt->ptm    = ptm_open(pt);
    pt->pts    = pts_open(pt);

    return vfs_open(pt->ptm, 0, 0);
}

void tty_init()
{
    struct file* ptmx = vfs_create_file();

    ptmx->ops.open    = ptmx_open;
    ptmx->flags       = FL_CHARDEV;
    
    vfs_addnode(ptmx, "/dev/ptmx");

    strcpy(ptmx->name, "ptmx");
    devfs_register(ptmx);
}

// TODO: use the /proc filesystem instead of this syscall
SYSCALL_DEFINE(ptsname, int fdno, char* buf, size_t n)
{
    PTRVALID(buf);
    FDVALID(fdno);

    struct fd* fd = task_curr()->fds[fdno];

    // TODO: struct file should hold flags like MASTER_PTY, DEVICE, etc (for fcntl() calls)
    // THIS IS DANGEROUS - IT MIGHT NOT BE A MASTER PTY
    struct pt* pt = fd->filp->device;

    if (strlen("/dev/pts/") + strlen(pt->pts->name) >= n) return -ERANGE;

    strcpy(buf, "/dev/pts/");
    strcpy(buf + strlen(buf), pt->pts->name);

    return 0;
}