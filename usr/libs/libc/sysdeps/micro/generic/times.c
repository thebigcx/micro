#include <sys/times.h>
#include <assert.h>

clock_t times(struct tms* buf)
{
    assert(!"times() not implemented!\n");
    return 0;
}