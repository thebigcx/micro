#include <libc/sysdeps-internal.h>
#include <sys/mount.h>
#include <errno.h>

int mount(const char* src, const char* dst,
          const char* fstype, unsigned long flags,
          const void* data)
{
    int e = sys_mount(src, dst, fstype, flags, data);

    if (e)
    {
        errno = e;
        return -1;
    }

    return 0;
}

int umount(const char* target)
{
    int e = sys_umount(target);
    
    if (e)
    {
        errno = e;
        return -1;
    }

    return 0;
}