#include <micro/vga.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <arch/pio.h>
#include <micro/devfs.h>

#define COLS 80
#define ROWS 25

static struct vga_char* buf;

struct PACKED vga_char
{
    uint8_t c;
    uint8_t col;
};

/*void enable_cursor(uint8_t start, uint8_t end)
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
}*/

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
    buf = (struct vga_char*)0xb8000;
    
    struct file* vga = vfs_create_file();
    
    vga->ops.read    = vga_read;
    vga->ops.write   = vga_write;
    vga->type        = S_IFCHR;
    vga->perms       = 0660;

    //strcpy(vga->name, "vga0");
    devfs_register(vga, "vga0");
}