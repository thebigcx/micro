#include <micro/dev.h>
#include <micro/vfs.h>
#include <micro/devfs.h>
#include <micro/stdlib.h>

ssize_t null_read(struct file* file, void* buf, off_t off, size_t size)
{
    return 0;
}

ssize_t null_write(struct file* file, const void* buf, off_t off, size_t size)
{
    return 0;
}

void init_devices()
{
    struct file* null = vfs_create_file();

    null->ops.read    = null_read;
    null->ops.write   = null_write;
    null->flags       = FL_CHARDEV;

    strcpy(null->name, "null");
    devfs_register(null);
}