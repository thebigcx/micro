// TODO: make kernel module

#include <micro/ps2.h>
#include <arch/reg.h>
#include <arch/pio.h>
#include <arch/ioapic.h>
#include <arch/lapic.h>
#include <arch/descs.h>
#include <micro/vfs.h>
#include <micro/list.h>
#include <micro/heap.h>
#include <micro/devfs.h>
#include <micro/stdlib.h>

static uint8_t queue[128];
static unsigned int count;

static void keyboard_handler(struct regs* r)
{
    uint8_t sc = inb(0x60);

    // TEMP
    //if (sc < 88)
        //tty_keypress(sc);

    if (count >= 128)
    {
        // TODO: use ringbuffer and begin writing at the start again
        return;
    }
    
    queue[count++] = sc;
}

ssize_t kb_read(struct file* file, void* buf, off_t off, size_t size)
{
    uint8_t* cbuf = buf;
    size_t bytes = 0;
    while (size-- && count)
    {
        *cbuf++ = queue[--count];
        bytes++;
    }

    return bytes;
}

void ps2_init()
{
    idt_set_handler(33, keyboard_handler);
    ioapic_redir(1, 33, DELIV_LOWEST);

    count = 0;

    struct file* kb = vfs_create_file();
    
    kb->flags       = FL_CHRDEV;
    kb->ops.read    = kb_read;

    strcpy(kb->name, "keyboard");
    devfs_register(kb);
}