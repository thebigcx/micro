#include <micro/sys.h>
#include <micro/vfs.h>

SYSCALL_DEFINE(mount, const char* src, const char* dst,
                      const char* fs, unsigned long flags,
                      const void* data)
{
    PTRVALID(src);
    PTRVALID(dst);
    PTRVALID(fs);

    PTRVALIDNULL(data);

    // TODO: vfs_mount_fs should return an error code
    return vfs_mount_fs(src, dst, fs, data);
}

SYSCALL_DEFINE(umount, const char* target)
{
    PTRVALID(target);
    return vfs_umount_fs(target);
}