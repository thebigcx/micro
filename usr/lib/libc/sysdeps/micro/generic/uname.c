#include <sys/utsname.h>
#include <libc/syscall.h>

int uname(struct utsname* buf)
{
    return SYSCALL_ERR(uname, buf);
}