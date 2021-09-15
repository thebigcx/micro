#include "ext2.h"

#include <micro/heap.h>
#include <micro/stdlib.h>
#include <micro/module.h>

#define SUPER_BLK 1
#define BGDS_BLK  2

#define INOSIZE(ino) (((size_t)ino.size_u << 32) | ino.size)

static ssize_t read_blocks(struct ext2_volume* vol, void* buf, uintptr_t blk, size_t cnt)
{
    return vfs_read(vol->device, buf, blk * vol->blksize, vol->blksize * cnt);
}

static ssize_t write_blocks(struct ext2_volume* vol, const void* buf, uintptr_t blk, size_t cnt)
{
    return vfs_write(vol->device, buf, blk * vol->blksize, vol->blksize * cnt);
}

static void ext2_rewrite_bgds(struct ext2_volume* vol)
{
    uint8_t* buf = kmalloc(vol->blksize);

    // TODO: support more than one block
    //for (uint32_t i = 0; i < vol->group_cnt; i++)
    {
        read_blocks(vol, buf, BGDS_BLK, 1);
        memcpy(buf, vol->groups, vol->group_cnt * sizeof(struct ext2_bgd));
        write_blocks(vol, buf, BGDS_BLK, 1);
    }
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

static void ext2_write_inode(struct ext2_volume* ext2, unsigned int num, struct ext2_inode* inode)
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
    memcpy(buf + blkoff, inode, sizeof(struct ext2_inode));
    write_blocks(ext2, buf, blk, 1);

    kfree(buf);
}

static struct file* dirent2file(struct ext2_volume* vol, struct file* parent, struct ext2_dirent* dirent)
{
    struct ext2_inode ino;
    ext2_read_inode(vol, dirent->inode, &ino);
    
    struct file* file = vfs_create_file();

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

#define INO_SIND 12 // Singly indirect
#define INO_DIND 13 // Doubly indirect
#define INO_TIND 14 // Triply indirect

static uint32_t ext2_inode_blk(struct ext2_volume* vol, struct ext2_inode* ino, uint32_t i)
{
    uint32_t bpp = vol->blksize / sizeof(uint32_t); // Blocks per pointer

    uint32_t sind = INO_SIND;         // Singly indirect start
    uint32_t dind = sind + bpp;       // Doubly indirect start
    uint32_t tind = dind + bpp * bpp; // Triply indirect start

    if (i < sind) // Direct data block
    {
        return ino->directs[i];
    }
    else if (i < dind) // Singly indirect
    {
        uint32_t* buf = kmalloc(vol->blksize);

        read_blocks(vol, buf, ino->sind, 1);
        uint32_t ret = buf[i - INO_SIND];
        kfree(buf);

        return ret;
    }
    else if (i < tind) // Doubly indirect
    {
        uint32_t* blk_ptrs = kmalloc(vol->blksize);
        uint32_t* buf = kmalloc(vol->blksize);

        read_blocks(vol, blk_ptrs, ino->dind, 1); // First indirect
        read_blocks(vol, buf, blk_ptrs[(i - dind) / bpp], 1); // Second indirect

        uint32_t ret = buf[(i - dind) % bpp];

        kfree(buf);
        kfree(blk_ptrs);

        return ret;
    }
    else // Triply indirect
    {
        uint32_t* sind_ptrs = kmalloc(vol->blksize);
        uint32_t* dind_ptrs = kmalloc(vol->blksize);
        uint32_t* buf = kmalloc(vol->blksize);
        
        read_blocks(vol, sind_ptrs, ino->tind,                   1); // First indirect
        read_blocks(vol, dind_ptrs, sind_ptrs[(i - dind) / bpp], 1); // Second indirect
        read_blocks(vol, buf,       dind_ptrs[(i - tind) / bpp], 1); // Third indirect

        uint32_t ret = buf[(i - tind) % bpp];

        kfree(buf);
        kfree(dind_ptrs);
        kfree(sind_ptrs);

        return ret;
    }
}

static void ext2_set_inode_blk(struct ext2_volume* vol, struct ext2_inode* ino, uint32_t i, uint32_t blk)
{
    uint32_t bpp = vol->blksize / sizeof(uint32_t); // Blocks per pointer

    uint32_t sind = INO_SIND;         // Singly indirect start
    uint32_t dind = sind + bpp;       // Doubly indirect start
    uint32_t tind = dind + bpp * bpp; // Triply indirect start

    printk("ext2: setting %d to %d\n", i, blk);

    if (i < sind)
    {
        ino->directs[i] = blk;
        ino->sector_cnt += vol->blksize / 512;
    }
    else if (i < dind)
    {
        uint32_t* buf = kmalloc(vol->blksize);

        read_blocks(vol, buf, ino->sind, 1);
        buf[i - INO_SIND] = blk;
        write_blocks(vol, buf, ino->sind, 1);

        kfree(buf);
    }
    else if (i < tind)
    {
    }
    else
    {
        // TODO: triply indirect blocks
    }
}

uint32_t ext2_alloc_blk(struct ext2_volume* vol)
{
    uint8_t* buf = kmalloc(vol->blksize);

    for (uint32_t i = 0; i < vol->group_cnt; i++)
    {
        uint32_t grp = vol->groups[i].block_bmp;
        read_blocks(vol, buf, grp, 1);

        for (uint32_t j = 0; j < vol->blksize; j++)
        {
            if (buf[j] == 0xff) continue;

            for (uint32_t k = 0; k < 8; k++)
            if ((buf[j] & (1 << k)) == 0)
            {
                buf[j] |= (1 << k);
                write_blocks(vol, buf, grp, 1);

                vol->groups[i].free_blocks--;
                ext2_rewrite_bgds(vol);

                kfree(buf);
                return i * vol->sb.blks_per_grp + j * 8 + k;
            }
        }
    }

    printk("ext2: out of blocks\n");
    kfree(buf);
    return 0;
}

void ext2_resize(struct file* file, size_t size)
{
    struct ext2_volume* vol = file->device;

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode, &ino);

    size_t diff = size - file->size;
    size_t cnt = diff % vol->blksize == 0
               ? diff / vol->blksize
               : diff / vol->blksize + 1;

    while (cnt--)
        ext2_set_inode_blk(vol, &ino, ino.sector_cnt / (vol->blksize / 512), ext2_alloc_blk(vol));

    ino.size = size & 0xffffffff;
    ino.size_u = size >> 32;

    ext2_write_inode(vol, file->inode, &ino);

    file->size = size;
}

ssize_t ext2_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct ext2_volume* vol = file->device;

    uint32_t startblk =  off         / vol->blksize; // Start block
    uint32_t modoff   =  off         % vol->blksize; // Byte offset of start block

    uint32_t endblk   = (size + off) / vol->blksize; // End block
    uint32_t modend   = (size + off) % vol->blksize; // Byte offset of end block

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode, &ino);

    uint8_t* fullbuf = kmalloc(vol->blksize);
    
    uint64_t ptroff = 0;
    for (uint32_t i = startblk; i <= endblk; i++)
    {
        read_blocks(vol, fullbuf, ext2_inode_blk(vol, &ino, i), 1);

        uint32_t start = 0;
        uint32_t size = vol->blksize;

        if (i == startblk)
        {
            start = modoff;
            size = vol->blksize - start;
        }
        if (i == endblk)
            size = modend;

        memcpy((void*)((uintptr_t)buf + ptroff), fullbuf + start, size);

        ptroff += size;
    }

    kfree(fullbuf);
    return size;
}

ssize_t ext2_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct ext2_volume* vol = file->device;

    if (file->size < off + size)
    {
        ext2_resize(file, off + size);
    }

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode, &ino);

    uint32_t startblk =  off         / vol->blksize; // Start block
    uint32_t modoff   =  off         % vol->blksize; // Byte offset of start block

    uint32_t endblk   = (size + off) / vol->blksize; // End block
    uint32_t modend   = (size + off) % vol->blksize; // Byte offset of end block

    uint8_t* fullbuf = kmalloc(vol->blksize);
    
    uint64_t ptroff = 0;
    for (uint32_t i = startblk; i <= endblk; i++)
    {
        read_blocks(vol, fullbuf, ext2_inode_blk(vol, &ino, i), 1);

        uint32_t start = 0;
        uint32_t wsize = vol->blksize;

        if (i == startblk)
        {
            start = modoff;
            wsize = vol->blksize - start;
        }
        if (i == endblk)
            wsize = modend;

        memcpy(fullbuf + start, (void*)((uintptr_t)buf + ptroff), wsize);

        write_blocks(vol, fullbuf, ext2_inode_blk(vol, &ino, i), 1);

        ptroff += wsize;
    }

    kfree(fullbuf);
    return size;
}

// TODO: move dirent parsing into one function
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

int ext2_readdir(struct file* dir, size_t idx, struct dirent* dirent)
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
        struct ext2_dirent* edirent = (struct ext2_dirent*)((uintptr_t)buf + offset);
        offset += edirent->size;

        if (!idx--)
        {
            strncpy(dirent->d_name, edirent->name, edirent->name_len);
            kfree(buf);
            return 1;
        }

        if (offset >= vol->blksize)
        {
            blk++; offset = 0;
            read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);
        }
    }

    kfree(buf);

    return 0;
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

    struct file* file = vfs_create_file();

    file->device      = vol;
    file->flags       = FL_MNTPT;
    file->inode       = 2;

    file->ops.read    = ext2_read;
    file->ops.write   = ext2_write;
    file->ops.find    = ext2_find;
    file->ops.readdir = ext2_readdir;
    file->ops.mkfile  = ext2_mkfile;
    file->ops.mkdir   = ext2_mkdir;
    file->ops.rm      = ext2_rm;

    return file;
}

void ext2_init()
{
    printk("loaded ext2 driver\n");
    vfs_register_fs("ext2", ext2_mount);
}

void ext2_fini()
{
    printk("finalizing ext2 driver\n");
}

struct modmeta meta =
{
    .init = ext2_init,
    .fini = ext2_fini,
    .name = "ext2fs"
};