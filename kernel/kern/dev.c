#include <micro/dev.h>
#include <micro/vfs.h>
#include <micro/devfs.h>
#include <micro/stdlib.h>

ssize_t null_read(struct fd* file, void* buf, off_t off, size_t size)
{
    return 0;
}

ssize_t null_write(struct fd* file, const void* buf, off_t off, size_t size)
{
    return 0;
}

void init_devices()
{
    struct new_file_ops ops = { .read = null_read, .write = null_write };
    devfs_register_chrdev(&ops, "null", 0666, NULL);
}