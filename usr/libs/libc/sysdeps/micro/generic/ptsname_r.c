#include <stdlib.h>
#include <libc/syscall.h>

int ptsname_r(int fd, char* buf, size_t buflen)
{
    return SYSCALL_ERR(ptsname, fd, buf, buflen);
}