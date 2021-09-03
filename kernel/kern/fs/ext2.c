#include <micro/ext2.h>
#include <micro/heap.h>
#include <micro/vfs.h>

struct file* ext2_init(const char* dev)
{
    struct ext2_volume* vol = kmalloc(sizeof(struct ext2_volume));
    vol->device = vfs_resolve(dev);

    struct file* file = kmalloc(sizeof(struct file));
    file->device = vol;
    file->flags = FL_DIR;
    return file;
}