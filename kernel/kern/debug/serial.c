#include <debug/syslog.h>
#include <stdlib.h>
#include <types.h>
#include <cpu.h>
#include <lock.h>
#include <pio.h>

#if DEBUG

#define PORT 0x3f8

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

static lock_t lock = 0;

void printk(const char* s, ...)
{
    char buffer[200];

    va_list list;
    va_start(list, s);
    snprintf(s, buffer, 200, list);
    va_end(list);

    LOCK(lock);
    
    char* f = buffer;
    while (*f != 0) serial_putch(*f++);

    UNLOCK(lock);
}

void printk_crit(const char* s, ...)
{
    char buffer[200];

    va_list list;
    va_start(list, s);
    snprintf(s, buffer, 200, list);
    va_end(list);

    char* f = buffer;
    while (*f != 0) serial_putch(*f++);
}

#else

void printk(const char* s, ...) {}
void printk_crit(const char* s, ...) {}

#endif