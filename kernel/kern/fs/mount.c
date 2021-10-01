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

    struct file target;
    int e;
    if ((e = vfs_resolve(dst, &target, 1))) return e;

    if (target.type != S_IFDIR) return -ENOTDIR;

    return vfs_mount_fs(src, dst, fs, data);
}

SYSCALL_DEFINE(umount, const char* target)
{
    PTRVALID(target);
    return vfs_umount_fs(target);
}