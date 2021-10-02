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

    struct inode target;
    int e;
    if ((e = vfs_resolve(dst, &target, 1))) return e;

    if (!S_ISDIR(target.mode)) return -ENOTDIR;

    return vfs_mount_fs(src, dst, fs, data);
}

SYSCALL_DEFINE(umount, const char* target)
{
    PTRVALID(target);
    return vfs_umount_fs(target);
}