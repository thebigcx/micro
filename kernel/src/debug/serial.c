#include <debug/syslog.h>
#include <types.h>

#if DEBUG

#define PORT 0x3f8

static void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

static uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static void serial_putch(char c)
{
    static int ready = 0;
    if (!ready)
    {
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x80);
        outb(PORT + 0, 0x03);
        outb(PORT + 1, 0x00);
        outb(PORT + 3, 0x03);
        outb(PORT + 2, 0xc7);
        outb(PORT + 4, 0x0b);
        outb(PORT + 4, 0x1e);
        outb(PORT + 0, 0xae);
        outb(PORT + 4, 0x0f);
        ready = 1;
    }

    // Wait for trasmit to be empty
    while ((inb(PORT + 5) & 0x20) == 0);
    outb(PORT, c); 
}

void __sputln(const char* s)
{
    while (*s != 0) serial_putch(*s++);
    serial_putch('\n');
}

#endif
