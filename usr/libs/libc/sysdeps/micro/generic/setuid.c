#include <unistd.h>
#include <assert.h>

int setuid(uid_t uid)
{
    assert(!"setuid() not implemented!\n");
    return 0;
}