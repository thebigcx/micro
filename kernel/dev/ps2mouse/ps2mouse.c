#include <micro/module.h>
#include <micro/debug.h>
#include <micro/platform.h>
#include <micro/vfs.h>
#include <micro/devfs.h>

#include <arch/pio.h>
#include <arch/reg.h>
#include <arch/descs.h>

// TODO: move into <micro/mouse.h> or similar for use in userspace
// Generic mouse packet
struct mouse_packet
{
    int32_t dx, dy;
};

struct PACKED ps2mouse_packet
{
    uint8_t attr;
    uint8_t dx, dy;
};

// x,y sign bits in byte #3
#define XSIGN(x) ((x & (1 << 6)) ? -1 : 1)
#define YSIGN(x) ((x & (1 << 5)) ? -1 : 1)

static struct mouse_packet s_queue[128];
static unsigned int s_count = 0;

void ps2mouse_handler(struct regs*)
{
    static int cycle = 0;
    static uint8_t packet[3];

    switch (cycle)
    {
        case 0:
            packet[0] = inb(0x60);
            if (!(packet[0] & 8)) break;
            
            cycle++;
            break;
            
        case 1:
            packet[1] = inb(0x60);
            cycle++;
            break;
       
        case 2:
            packet[2] = inb(0x60);
            cycle = 0;
            
            s_queue[s_count++] = (struct mouse_packet)
            {
                .dx = packet[1] * XSIGN(packet[0]),
                .dy = packet[2] * YSIGN(packet[0])
            };
            
            // TODO: ringbuffer (wrap-around)
            if (s_count == 128) s_count = 0;
            
            break;
    }
}

ssize_t ps2mouse_read(struct file*, void* buf, off_t, size_t size)
{
    struct mouse_packet* packs = buf;
    size_t bytes;
    
    for (bytes = 0; bytes < size && s_count; bytes += sizeof(struct mouse_packet))
        *packs++ = s_queue[s_count--];
    
    return bytes;
}

static void wait()
{
    unsigned int time = 100000;
    while (time-- && inb(0x64) & 2);
}

static void wait_input()
{
    unsigned int time = 100000;
    while (time-- && !(inb(0x64) & 1));
}

static uint8_t read8()
{
    wait_input();
    return inb(0x60);
}

static void write8(uint8_t val)
{
    wait();
    outb(0x64, 0xd4);
    wait();
    outb(0x60, val);
}

void ps2mouse_init()
{
    printk("loaded PS/2 mouse driver\n");
    
    wait();
    outb(0x64, 0xa8);
    outb(0x64, 0xff);
    
    wait();
    outb(0x64, 0x20);
    wait_input();
    uint8_t status = ((inb(0x60) & ~0x20) | 2);
    wait();
    outb(0x64, 0x60);
    wait();
    outb(0x60, status);
    
    write8(0xf6);
    read8();
    write8(0xf4);
    read8();

    register_irq_handler(12, ps2mouse_handler);

    struct file_ops ops = { .read = ps2mouse_read };
    devfs_register_chrdev(&ops, "mouse", 0660, NULL);
}

void ps2mouse_fini()
{
    printk("finalizing PS/2 mouse driver\n");
}

struct modmeta meta =
{
    .init = ps2mouse_init,
    .fini = ps2mouse_fini,
    .name = "ps2mouse"
};
