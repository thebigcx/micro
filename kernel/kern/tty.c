#include <micro/tty.h>
#include <micro/vfs.h>
#include <micro/ioctls.h>
#include <micro/termios.h>
#include <micro/errno.h>
#include <micro/heap.h>
#include <micro/ps2.h>
#include <micro/fb.h>

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
        vga_putc(*cbuf++);
        //fb_putch(*cbuf++, 0xffffffff, 0x0);
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

void tty_init()
{
    struct file* tty = kmalloc(sizeof(struct file));
    tty->ops.read = tty_read;
    tty->ops.write = tty_write;
    tty->ops.ioctl = tty_ioctl;
    tty->flags = FL_CHARDEV;
    vfs_addnode(tty, "/dev/tty");
}