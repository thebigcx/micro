#include "ps2.h"

#include <micro/module.h>
#include <micro/debug.h>
#include <arch/pio.h>
#include <micro/stdlib.h>
#include <micro/devfs.h>
#include <micro/vfs.h>
#include <arch/pio.h>
#include <arch/descs.h>
#include <arch/reg.h>

#define REG_DATA 0x60
#define REG_CMD  0x64
#define REG_STAT 0x64

#define STAT_OUTBUF  (1 << 0)
#define STAT_INBUF   (1 << 1)
#define STAT_SYS     (1 << 2)
#define STAT_CMD     (1 << 3)
#define STAT_TIMEOUT (1 << 6)
#define STAT_PARITY  (1 << 7)

static uint8_t kbqueue[128];
static unsigned int kbcount = 0;

void irq1_handler(struct regs*)
{
    uint8_t sc = inb(REG_DATA);

    if (kbcount >= 128)
        return;
        // TODO: use ringbuffer and begin writing at the start again
    
    kbqueue[kbcount++] = sc;
}

ssize_t keyboard_read(struct file*, void* buf, off_t, size_t size)
{
    size_t i;
    for (i = 0; i < size && kbcount; i++)
        *((uint8_t*)buf) = kbqueue[--kbcount];
    
    return i;
}

static struct file_ops kbops = { .read = keyboard_read };

void init_keyboard()
{
    ps2_write2(0xd2, 0xff);

    register_irq_handler(1, irq1_handler);
    devfs_register_chrdev(&kbops, "keyboard", 0660, NULL);
}

// Wait before reading
void wait_read()
{
    unsigned int time = 100000;
    while (time-- && !(inb(REG_CMD) & STAT_OUTBUF));
}

// Wait before writing
void wait_write()
{
    unsigned int time = 100000;
    while (time-- && (inb(REG_CMD) & STAT_INBUF));
}

// Send command with extra byte
void ps2_write2(uint8_t cmd, uint8_t val)
{
    wait_write();
    outb(REG_CMD, cmd);
    wait_write();
    outb(REG_DATA, val);
}

void ps2_write(uint8_t cmd)
{
    wait_write();
    outb(REG_CMD, cmd);
}

uint8_t ps2_read()
{
    wait_read();
    return inb(REG_DATA);
}

void ps2_init()
{
    printk("loaded PS/2 driver\n");

    // Disable ports
    ps2_write(0xad);
    ps2_write(0xa7);

    // Flush buffer
    inb(REG_DATA);

    // Enable ports
    ps2_write(0xae);
    ps2_write(0xa8);

    init_keyboard();
    //init_mouse();
}

void ps2_fini()
{
    printk("finalizing PS/2 driver\n");
}

struct modmeta meta =
{
    .init = ps2_init,
    .fini = ps2_fini,
    .name = "ps2"
};
