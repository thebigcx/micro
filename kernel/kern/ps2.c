// TODO: make kernel module

#include <micro/ps2.h>
#include <arch/reg.h>
#include <arch/pio.h>
#include <arch/ioapic.h>
#include <arch/lapic.h>
#include <micro/vfs.h>
#include <micro/list.h>

static struct list queue;

static void keyboard_handler(struct regs* r)
{
    uint8_t sc = inb(0x60);
    list_push_back(&queue, (void*)sc);
}

ssize_t kb_read(struct file* file, void* buf, off_t off, size_t size)
{
    size_t bytes = 0;
    while (size-- && queue.size)
    {
        *((char*)buf++) = (uint8_t)list_dequeue(&queue);
        bytes++;
    }

    return bytes;
}

void ps2_init()
{
    idt_set_handler(33, keyboard_handler);
    ioapic_redir(1, 33, DELIV_LOWEST);

    queue = list_create();

    struct file* kb = kmalloc(sizeof(struct file));
    kb->flags = FL_CHARDEV;
    kb->ops.read = kb_read;
    vfs_addnode(kb, "/dev/keyboard");
}