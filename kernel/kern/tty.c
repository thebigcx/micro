#include <micro/tty.h>
#include <micro/vfs.h>
#include <micro/ioctls.h>
#include <micro/termios.h>
#include <micro/errno.h>
#include <micro/heap.h>
#include <micro/ps2.h>
#include <micro/fb.h>
#include <micro/ringbuf.h>

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

static struct ringbuf* tty_buffer = NULL;

static char tty_line_buffer[128];
static size_t tty_line_idx = 0;

void tty_keypress(int scancode)
{
    char c = ascii[scancode];
    //vga_putc(c);
    fb_putch(c, 0xffffffff, 0);

    if (c == '\b')
    {
        if (tty_line_idx) tty_line_idx--;
        return;
    }
    
    tty_line_buffer[tty_line_idx++] = c;

    if (c == '\n')
    {
        ringbuf_write(tty_buffer, tty_line_buffer, tty_line_idx);
        tty_line_idx = 0;
    }
}

// FIXME: this is terrible - but at least it works
ssize_t tty_read(struct file* file, void* buf, off_t off, size_t size)
{
    ssize_t bytes = ringbuf_size(tty_buffer);

    if (bytes <= 0) return 0;
    if (size < bytes) bytes = size;

    ringbuf_read(tty_buffer, buf, bytes);

    return bytes;
}

ssize_t tty_write(struct file* file, const void* buf, off_t off, size_t size)
{
    const char* cbuf = buf;
    for (size_t i = 0; i < size; i++)
    {
        //vga_putc(cbuf[i]);
        fb_putch(cbuf[i], 0xffffffff, 0);
    }
    
    return size;
}

int tty_set(struct file* file, struct termios* t)
{
    return 0;
}

int tty_ioctl(struct file* file, unsigned long req, void* argp)
{
    switch (req)
    {
        case TIOCGWINSZ:
        {
            // TODO: this is bad and hacky
            struct winsize* ws = argp;
            ws->ws_row = 25;
            ws->ws_col = 80;
            break;
        }
        case TCSETS:
        case TCSETSW:
        case TCSETSF:
            return tty_set(file, (struct termios*)argp);

        default:
        {
            return -EINVAL;
        }
    }

    return 0;
}

ssize_t pts_read(struct file* file, void* buf, off_t off, size_t size)
{

}

ssize_t pts_write(struct file* file, const void* buf, off_t off, size_t size)
{
    
}

ssize_t ptm_read(struct file* file, void* buf, off_t off, size_t size)
{
    
}

ssize_t ptm_write(struct file* file, const void* buf, off_t off, size_t size)
{
    
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

    pt->buffer = ringbuf_create(1024);
    pt->ptm    = ptm_open(pt);
    pt->pts    = pts_open(pt);

    return vfs_open(pt->ptm, 0, 0);
}

void tty_init()
{
    // TODO: temporary
    tty_buffer = ringbuf_create(1024);

    struct file* tty = vfs_create_file();

    tty->ops.read    = tty_read;
    tty->ops.write   = tty_write;
    tty->ops.ioctl   = tty_ioctl;
    tty->flags       = FL_CHARDEV;

    vfs_addnode(tty, "/dev/tty");

    // -- up to here

    struct file* ptmx = vfs_create_file();

    ptmx->ops.open    = ptmx_open;
    ptmx->flags       = FL_CHARDEV;
    
    vfs_addnode(ptmx, "/dev/ptmx");

}