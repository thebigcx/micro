#include <micro/sys.h>
#include <arch/pio.h>

SYSCALL_DEFINE(reboot)
{
    outw(0x604, 0x2000);
    return 0;
}