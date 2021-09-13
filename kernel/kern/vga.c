#include <micro/vga.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <arch/pio.h>

#define COLS 80
#define ROWS 25

static struct vga_char* buf;
static unsigned int cx = 0, cy = 0;
static unsigned int col = (VGA_BLACK << 4) | VGA_WHITE;

struct __attribute__((packed)) vga_char
{
    uint8_t c;
    uint8_t col;
};

void vga_set_addr(uintptr_t addr)
{
    buf = (struct vga_char*)addr;
    enable_cursor(0, 15);
}

void enable_cursor(uint8_t start, uint8_t end)
{
    outb(0x3d4, 0x0a);
    outb(0x3d5, (inb(0x3d5) & 0xc0) | start);

    outb(0x3d4, 0x0b);
    outb(0x3d5, (inb(0x3d5) & 0xe0) | end);   
}

void update_cursor()
{
    uint16_t pos = cy * COLS + cx;

    outb(0x3d4, 0x0f);
    outb(0x3d5, (uint8_t)(pos & 0xff));
    outb(0x3d4, 0x0e);
    outb(0x3d5, (uint8_t)((pos >> 8) & 0xff));
}

void clear_row(unsigned int row)
{
    struct vga_char ch =
    {
        .c = ' ',
        .col = col
    };

    for (unsigned int x = 0; x < COLS; x++)
    {
        buf[x + row * COLS] = ch;
    }
}

void newline()
{
    cx = 0;
    
    if (cy < ROWS - 1)
    {
        cy++;
        return;
    }

    for (unsigned int y = 1; y < ROWS; y++)
    for (unsigned int x = 0; x < COLS; x++)
    {
        buf[x + (y - 1) * COLS] = buf[x + y * COLS];
    }

    clear_row(ROWS - 1);
}

void backspace()
{
    if (cx == 0)
    {
        cx = ROWS - 1;
        cy--; // TODO: scroll up if need be
    }
    cx--;

    buf[cx + cy * COLS] = (struct vga_char)
    {
        .c = ' ',
        .col = col
    };
}

#define TABSIZE 4

void tab()
{
    unsigned int near = cx % TABSIZE;
    if (!near) near += TABSIZE;

    cx += near;
}

void vga_putc(char c)
{
    if (c == '\n')
    {
        newline();
        goto done;
    }
    if (c == '\b')
    {
        backspace();
        goto done;
    }
    if (c == '\t')
    {
        tab();
        goto done;
    }

    struct vga_char ch =
    {
        .c = (uint8_t)c,
        .col = col
    };

    buf[cx + cy * COLS] = ch;

    cx++;
    if (cx == COLS)
    {
        newline();
    }

done:
    update_cursor();
}

void vga_set_color(int fg, int bg)
{
    col = (bg << 4) | fg;
}

ssize_t vga_read(struct file* file, void* ptr, off_t off, size_t size)
{
    if (off + size >= ROWS * COLS)
        size -= ((off + size) - ROWS * COLS);

    memcpy(ptr, buf + off, size);

    return size;
}

ssize_t vga_write(struct file* file, const void* ptr, off_t off, size_t size)
{
    if (off + size >= ROWS * COLS)
        size -= ((off + size) - ROWS * COLS);

    memcpy(buf + off, ptr, size * sizeof(struct vga_char));

    return size;
}

void vga_init()
{
    struct file* vga = vfs_create_file();
    vga->ops.read = vga_read;
    vga->ops.write = vga_write;
    vga->flags = FL_CHARDEV;
    vfs_addnode(vga, "/dev/vga0");
}