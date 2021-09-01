#include <panic.h>
#include <pio.h>

static void shutdown()
{
    outw(0x604, 0x2000);
}

void panic(const char* msg)
{
    printk("Kernel panic: %s\n", msg);
    shutdown();
}