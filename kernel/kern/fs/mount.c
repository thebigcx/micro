#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/try.h>

SYSCALL_DEFINE(mount, const char* src, const char* dst,
                      const char* fs, unsigned long flags,
                      const void* data)
{
    PTRVALID(src);
    PTRVALID(dst);
    PTRVALID(fs);

    PTRVALIDNULL(data);

    //struct inode target;
    //TRY(vfs_resolve(dst, &target, 1));

    //if (!S_ISDIR(target.mode)) return -ENOTDIR;

    return vfs_mount_fs(src, dst, fs, data);
}

SYSCALL_DEFINE(umount, const char* target)
{
    PTRVALID(target);
    return vfs_umount_fs(target);
}
