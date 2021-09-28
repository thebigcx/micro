#include <fcntl.h>
#include <assert.h>

int fcntl(int fd, int cmd, ...)
{
    assert(!"fcntl() not implemented!\n");
    return 0;
}