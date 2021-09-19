#include <micro/sys.h>

// TODO: implement
SYSCALL_DEFINE(time, time_t* time)
{
    PTRVALID(time);
    *time = 0;
    return 0;
}