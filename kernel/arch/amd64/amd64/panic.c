#include <arch/panic.h>
#include <arch/pio.h>
#include <micro/debug.h>

static void shutdown()
{
    outw(0x604, 0x2000);
    asm volatile ("hlt");
}

void panic(const char* msg)
{
    printk("Kernel panic: %s\n", msg);
    shutdown();
}