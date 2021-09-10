#include <micro/ext2.h>
#include <micro/heap.h>
#include <micro/stdlib.h>

// TODO: compile as kernel module and load at runtime

#define SUPER_BLK 1

#define INOSIZE(ino) (((size_t)ino.size_u << 32) | ino.size)

static ssize_t read_blocks(struct ext2_volume* vol, void* buf, uintptr_t blk, size_t cnt)
{
    return vfs_read(vol->device, buf, blk * vol->blksize, vol->blksize * cnt);
}

static void ext2_read_inode(struct ext2_volume* ext2, unsigned int num, struct ext2_inode* inode)
{
    uint8_t* buf = kmalloc(ext2->blksize);

    // Inode block group descriptor
    size_t grpoff       = (num - 1) % ext2->sb.inodes_per_grp;
    size_t grp          = (num - 1) / ext2->sb.inodes_per_grp;

    // Inode table
    uintptr_t table     = ext2->groups[grp].inode_tbl;

    // Offset in inode table (Block, Offset in block)
    uintptr_t blk       = (table * ext2->blksize + grpoff * ext2->sbext.inode_sz) / ext2->blksize;
    unsigned int blkoff = (grpoff * ext2->sbext.inode_sz) % ext2->blksize;

    read_blocks(ext2, buf, blk, 1);
    memcpy(inode, buf + blkoff, sizeof(struct ext2_inode));

    kfree(buf);
}

static struct file* dirent2file(struct ext2_volume* vol, struct file* parent, struct ext2_dirent* dirent)
{
    struct ext2_inode ino;
    ext2_read_inode(vol, dirent->inode, &ino);
    
    struct file* file = kmalloc(sizeof(struct file));

    file->ops.read    = ext2_read;
    file->ops.write   = ext2_write;
    file->ops.find    = ext2_find;
    file->ops.readdir = ext2_readdir;
    file->ops.mkfile  = ext2_mkfile;
    file->ops.mkdir   = ext2_mkdir;
    file->ops.rm      = ext2_rm;

    file->parent      = parent;
    file->inode       = dirent->inode;
    file->size        = INOSIZE(ino);
    file->device      = vol;

    strncpy(file->name, dirent->name, dirent->name_len);

    if (ino.mode & INODE_FILE)
        file->flags = FL_FILE;
    else if (ino.mode & INODE_DIR)
        file->flags = FL_DIR;

    return file;
}

static uint32_t ext2_inode_blk(struct ext2_volume* vol, struct ext2_inode* ino, unsigned int i)
{
    // TODO: indirect blocks
    return ino->blocks[i];
}

ssize_t ext2_read(struct file* file, void* buf, off_t off, size_t size)
{
    return -1;
}

ssize_t ext2_write(struct file* file, const void* buf, off_t off, size_t size)
{
    return -1;
}


// TODO: return error codes somehow
struct file* ext2_find(struct file* dir, const char* name)
{
    struct ext2_volume* vol = dir->device;

    struct ext2_inode ino;
    ext2_read_inode(vol, dir->inode, &ino);

    uintptr_t offset = 0;
    uintptr_t blk    = 0;

    void* buf = kmalloc(vol->blksize);
    read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);

    while (offset + blk * vol->blksize < INOSIZE(ino))
    {
        struct ext2_dirent* dirent = (struct ext2_dirent*)((uintptr_t)buf + offset);
        offset += dirent->size;

        if (strlen(name) == dirent->name_len && !strncmp(name, dirent->name, dirent->name_len))
        {
            struct file* file = dirent2file(vol, dir, dirent);
            kfree(buf);
            return file;
        }

        if (offset >= vol->blksize)
        {
            blk++; offset = 0;
            read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);
        }
    }

    kfree(buf);

    return NULL;
}

int ext2_readdir(struct file* dir, size_t size, struct dirent* dirent)
{
    return -1;
}

void ext2_mkfile(struct file* dir, const char* name)
{

}

void ext2_mkdir(struct file* dir, const char* name)
{

}

void ext2_rm(struct file* dir, const char* name)
{

}

static struct file* ext2_mount(const char* dev, void* data)
{
    struct ext2_volume* vol = kmalloc(sizeof(struct ext2_volume));
    vol->device = vfs_resolve(dev);

    void* buf = kmalloc(512);

    vfs_read(vol->device, buf, SUPER_BLK * 1024, 512); // FIXME: here we assume that the blocks are 1024 in size
    memcpy(&vol->sb, buf, sizeof(struct ext2_sb) + sizeof(struct ext2_sbext));

    kfree(buf);

    vol->blksize = 1024u << vol->sb.log_block_sz;

    vol->group_cnt = (vol->sb.blk_cnt % vol->sb.blks_per_grp)
                   ? (vol->sb.blk_cnt / vol->sb.blks_per_grp + 1)
                   : (vol->sb.blk_cnt / vol->sb.blks_per_grp);

    vol->groups = kmalloc(sizeof(struct ext2_bgd) * vol->group_cnt + vol->blksize);

    read_blocks(vol, vol->groups, SUPER_BLK + 1, vol->group_cnt * sizeof(struct ext2_bgd) / vol->blksize + 1);

    struct file* file = kmalloc(sizeof(struct file));

    file->device   = vol;
    file->flags    = FL_MNTPT;
    file->inode    = 2;
    file->ops.find = ext2_find;

    // TESTS
    //struct ext2_inode ino;
    //ext2_read_inode(vol, 2, &ino);
    struct file* init = ext2_find(file, "init");
    for (;;);

    return file;
}

void ext2_init()
{
    vfs_register_fs("ext2", ext2_mount);
}