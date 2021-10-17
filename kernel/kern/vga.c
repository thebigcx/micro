#include <micro/vga.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <arch/pio.h>
#include <arch/mmu.h>
#include <micro/task.h>
#include <micro/devfs.h>
#include <micro/errno.h>
#include <micro/vmmap.h>

#define COLS 80
#define ROWS 25

#define VGASCURSPOS 0

static uintptr_t addr;

struct PACKED vga_char
{
    uint8_t c;
    uint8_t col;
};

struct cursor
{
    uint8_t x, y;
};

int vga_ioctl(struct file* file, int req, void* argp)
{
    switch (req)
    {
        case VGASCURSPOS:
        {
            struct cursor* curs = argp;
            uint16_t pos = curs->y * 80 + curs->x;

            outb(0x3d4, 0x0f);
            outb(0x3d5, (uint8_t)(pos & 0xff));
            outb(0x3d4, 0x0e);
            outb(0x3d5, (uint8_t)((pos >> 8) & 0xff));
            
            return 0;
        }
    }
    
    return -EINVAL;
}

int vga_mmap(struct file* file, struct vm_area* area)
{
    printk("vga_mmap(%x-%x)\n", area->base, area->end);
    for (uintptr_t i = 0; i < (area->end - area->base) / PAGE4K; i++)
    {
        printk("%x-%x\n", area->base + i * PAGE4K, addr + i * PAGE4K);
        mmu_map(task_curr()->vm_map->pagemap, area->base + i * PAGE4K, addr + i * PAGE4K, PAGE_PR | PAGE_RW | PAGE_USR);
    }
 
    return 0;
}

void vga_init()
{
    addr = 0xb8000;

    outb(0x3d4, 0x0a);
    outb(0x3d5, (inb(0x3d5) & 0xc0) | 0);
     
    outb(0x3d4, 0x0b);
    outb(0x3d5, (inb(0x3d5) & 0xe0) | 15);

    struct file_ops ops = { .mmap = vga_mmap, .ioctl = vga_ioctl };
    devfs_register_chrdev(&ops, "vga0", 0660, NULL);
}
