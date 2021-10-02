#include <micro/module.h>
#include <micro/debug.h>
#include <arch/reg.h>
#include <arch/descs.h>
#include <arch/lapic.h>
#include <arch/ioapic.h>
#include <arch/pio.h>
#include <micro/vfs.h>
#include <micro/devfs.h>
#include <micro/stdlib.h>

#define PS2KB_IRQ 1

static uint8_t queue[128];
static unsigned int count = 0;

void ps2kb_handler(struct regs* r)
{
    (void)r;

    uint8_t sc = inb(0x60);

    if (count >= 128)
        return;
        // TODO: use ringbuffer and begin writing at the start again
    
    queue[count++] = sc;
}

ssize_t kb_read(struct fd* file, void* buf, off_t off, size_t size)
{
    (void)file; (void)off;

    uint8_t* cbuf = buf;
    size_t bytes = 0;
    while (size-- && count)
    {
        *cbuf++ = queue[--count];
        bytes++;
    }

    return bytes;
}

void ps2kb_init()
{
    printk("loaded PS/2 keyboard driver\n");

    register_irq_handler(1, ps2kb_handler);

    struct new_file_ops ops = { .read = kb_read };
    devfs_register_chrdev(&ops, "keyboard", 0660, NULL);

    count = 0;
}

void ps2kb_fini()
{
    printk("finalizing PS/2 keyboard driver\n");
}

struct modmeta meta =
{
    .init = ps2kb_init,
    .fini = ps2kb_fini,
    .name = "ps2kb"
};