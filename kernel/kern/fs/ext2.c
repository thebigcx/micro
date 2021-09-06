#include <micro/ext2.h>
#include <micro/heap.h>
#include <micro/vfs.h>

// TODO: compile as kernel module and load at runtime

/*static void ext2_read_inode(struct ext2_volume* ext2, unsigned int num, struct ext2_inode* inode)
{
    void* buf = kmalloc(512);
    vfs_read(ext2->device, buf, ext2_inode_lba(ext2, num) * 512, 512);
    memcpy(inode, (buf + (ext2_inode_bgi(ext2, num) * ext2->superext.inode_sz) % 512))
}*/

/*static struct file* ext2vol_find(struct file* dir, const char* name)
{
    struct ext2_volume* vol = dir->device;

    
}

static struct file* ext2_mount(const char* dev)
{
    struct ext2_volume* volume = kmalloc(sizeof(struct ext2_volume));
    volume->device = vfs_resolve(dev);

    struct file* file = kmalloc(sizeof(struct file));
    file->device = volume;
    file->flags = FL_MNTPT;

    return file;
}

void ext2_init()
{
    vfs_register_fs("ext2", ext2_mount);
}*/