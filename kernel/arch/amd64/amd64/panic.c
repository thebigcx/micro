#include <panic.h>
#include <pio.h>

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