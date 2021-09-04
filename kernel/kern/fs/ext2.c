#include <micro/ext2.h>
#include <micro/heap.h>
#include <micro/vfs.h>

// TODO: compile as kernel module and load at runtime

struct file* ext2_mount(const char* dev)
{
    struct ext2_volume* volume = kmalloc(sizeof(struct ext2_volume));

    struct file* file = kmalloc(sizeof(struct file));
    file->device = volume;
    file->flags = FL_MNTPT;

    return file;
}

void ext2_init()
{
    vfs_register_fs("ext2", ext2_mount);
}