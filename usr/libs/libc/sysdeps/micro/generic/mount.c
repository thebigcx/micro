#include <unistd.h>
#include <libc/syscall.h>

int mount(const char* src, const char* dst,
          const char* fstype, unsigned long flags,
          const void* data)
{
    return SYSCALL_ERR(mount, src, dst, fstype, flags, data);
}