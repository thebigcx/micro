#include "ext2.h"

#include <micro/heap.h>
#include <micro/stdlib.h>
#include <micro/module.h>
#include <micro/debug.h>

#define SUPER_BLK 1
#define BGDS_BLK  2

#define INOSIZE(ino) (((size_t)((ino).size_u) << 32) | (ino).size)

static ssize_t read_blocks(struct ext2_volume* vol, void* buf,
                           uintptr_t blk, size_t cnt)
{
    return vfs_read(vol->device, buf, blk * vol->blksize, vol->blksize * cnt);
}

static ssize_t write_blocks(struct ext2_volume* vol, const void* buf,
                            uintptr_t blk, size_t cnt)
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

static void ext2_read_inode(struct ext2_volume* ext2, unsigned int num,
                            struct ext2_inode* inode)
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

static void ext2_write_inode(struct ext2_volume* ext2, unsigned int num,
                             struct ext2_inode* inode)
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

static struct file* dirent2file(struct ext2_volume* vol, struct file* parent,
                                struct ext2_dirent* dirent)
{
    struct ext2_inode ino;
    ext2_read_inode(vol, dirent->inode, &ino);
    
    struct file* file  = vfs_create_file();

    file->ops.read     = ext2_read;
    file->ops.write    = ext2_write;
    file->ops.find     = ext2_find;
    file->ops.getdents = ext2_getdents;
    file->ops.mkfile   = ext2_mkfile;
    file->ops.mkdir    = ext2_mkdir;
    file->ops.unlink   = ext2_unlink;

    file->parent       = parent;
    file->inode        = dirent->inode;
    file->size         = INOSIZE(ino);
    file->device       = vol;
    file->links        = ino.link_cnt;

    file->atime        = ino.last_access;
    file->ctime        = ino.creation_time;
    file->mtime        = ino.last_mod_time;

    file->flags        = ino.mode & 0xf000;

    strncpy(file->name, dirent->name, dirent->name_len);

    return file;
}

#define INO_SIND 12 // Singly indirect
#define INO_DIND 13 // Doubly indirect
#define INO_TIND 14 // Triply indirect

static uint32_t ext2_inode_blk(struct ext2_volume* vol, struct ext2_inode* ino,
                               uint32_t i)
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

static void ext2_set_inode_blk(struct ext2_volume* vol, struct ext2_inode* ino,
                               uint32_t i, uint32_t blk)
{
    uint32_t bpp = vol->blksize / sizeof(uint32_t); // Blocks per pointer

    uint32_t sind = INO_SIND;         // Singly indirect start
    uint32_t dind = sind + bpp;       // Doubly indirect start
    uint32_t tind = dind + bpp * bpp; // Triply indirect start

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

static uint8_t ext2_dirent_type(unsigned int type)
{
    switch (type)
    {
        case FL_FILE:
            return DIRENT_FILE;
        case FL_DIR:
            return DIRENT_DIR;
        case FL_CHRDEV:
            return DIRENT_CHARDEV;
        case FL_BLKDEV:
            return DIRENT_BLOCKDEV;
        case FL_FIFO:
            return DIRENT_FIFO;
        case FL_SOCKET:
            return DIRENT_SOCKET;
        case FL_SYMLINK:
            return DIRENT_SYMLINK;
    }

    printk("ext2: unknown file type %d\n", type);
    return DIRENT_UNK;
}

static uint16_t ext2_inode_type(unsigned int type)
{
    switch (type)
    {
        case FL_FILE:
            return INODE_FILE;
        case FL_DIR:
            return INODE_DIR;
        case FL_CHRDEV:
            return INODE_CHARDEV;
        case FL_BLKDEV:
            return INODE_BLOCKDEV;
        case FL_FIFO:
            return INODE_FIFO;
        case FL_SOCKET:
            return INODE_SOCKET;
        case FL_SYMLINK:
            return INODE_SYMLINK;
    }

    printk("ext2: unknown file type %d\n", type);
    return 0;
}

uint32_t ext2_alloc_blk(struct ext2_volume* vol)
{
    uint8_t* buf = kmalloc(vol->blksize);

    for (uint32_t i = 0; i < vol->group_cnt; i++)
    {
        if (!vol->groups[i].free_blocks) continue;

        read_blocks(vol, buf, vol->groups[i].block_bmp, 1);

        // Find the first unset bit
        size_t j = 0;
        for (; j < vol->blksize * 8 && (buf[j / 8] & (1 << (j % 8))); j++);
        if (j == vol->blksize * 8) continue;
        
        // Set it and return
        buf[j / 8] |= (1 << (j % 8));
        write_blocks(vol, buf, vol->groups[i].block_bmp, 1);

        vol->groups[i].free_blocks--;
        ext2_rewrite_bgds(vol);

        kfree(buf);
        return i * vol->sb.blks_per_grp + j;
    }

    printk("ext2: out of blocks\n");
    kfree(buf);
    return 0;
}

uint32_t ext2_alloc_inode(struct ext2_volume* vol)
{
    uint8_t* buf = kmalloc(vol->blksize);

    for (uint32_t i = 0; i < vol->group_cnt; i++)
    {
        if (!vol->groups[i].free_inodes) continue;

        read_blocks(vol, buf, vol->groups[i].inode_bmp, 1);

        // Find the first unset bit
        size_t j = 0;
        for (; j < vol->blksize * 8 && (buf[j / 8] & (1 << (j % 8))); j++);
        if (j == vol->blksize * 8) continue;
        
        // Set it and return
        buf[j / 8] |= (1 << (j % 8));
        write_blocks(vol, buf, vol->groups[i].inode_bmp, 1);

        vol->groups[i].free_inodes--;
        ext2_rewrite_bgds(vol);

        kfree(buf);
        return i * vol->sb.inodes_per_grp + j + 1; // Inode addresses start at 1
    }

    printk("ext2: could not allocate a free inode\n");
    kfree(buf);

    return 0;
}

ssize_t ext2_getdents(struct file* dir, off_t off, size_t n, struct dirent* dirp)
{
    struct ext2_volume* vol = dir->device;

    struct ext2_inode ino;
    ext2_read_inode(vol, dir->inode, &ino);

    // TODO: maybe this is unnecessary
    size_t dentidx = 0;
    ssize_t bytes = 0;
    size_t idx = 0;

    uintptr_t offset = 0;
    uintptr_t blk    = 0;

    void* buf = kmalloc(vol->blksize);
    read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);

    while (offset + blk * vol->blksize < INOSIZE(ino))
    {
        struct ext2_dirent* dirent = (struct ext2_dirent*)((uintptr_t)buf + offset);
        offset += dirent->size;

        if (idx++ < (size_t)off) continue;

        strncpy(dirp[dentidx].d_name, dirent->name, dirent->name_len);
        dentidx++;
        bytes += dirent->size;
        if (idx >= off + n)
        {
            kfree(buf);
            return bytes;
        }

        if (offset >= vol->blksize)
        {
            blk++; offset = 0;
            read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);
        }
    }

    kfree(buf);
    return bytes;
}

void ext2_resize(struct file* file, size_t size)
{
    struct ext2_volume* vol = file->device;

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode, &ino);

    // Get number of blocks needed to resize
    size_t diff = size - file->size;
    size_t cnt = diff % vol->blksize == 0
               ? diff / vol->blksize
               : diff / vol->blksize + 1;

    while (cnt--)
    {
        // Allocate blocks on the end
        uint32_t i = ino.sector_cnt / (vol->blksize / 512);
        ext2_set_inode_blk(vol, &ino, i, ext2_alloc_blk(vol));
    }

    ino.size   = size & 0xffffffff;
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

void ext2_init_inode(struct ext2_volume* vol, struct ext2_inode* ino, struct file* file)
{
    memset(ino, 0, sizeof(struct ext2_inode));

    //ino->mode |= perms & 0xfff;
    ino->mode |= ext2_inode_type(file->flags);
    ino->size = vol->blksize;

    ino->sector_cnt = vol->blksize / 512;
    ino->link_cnt = 1;
    
    ext2_set_inode_blk(vol, ino, 0, ext2_alloc_blk(vol)); // One block to start off with
}

void ext2_mkentry(struct file* dir, struct file* file)
{
    struct ext2_volume* vol = dir->device;

    uint32_t inonum = ext2_alloc_inode(vol);

    file->inode = inonum;

    // Initialize the inode and write to disk
    struct ext2_inode ino;
    ext2_read_inode (vol, inonum, &ino);
    ext2_init_inode (vol, &ino, file);
    ext2_write_inode(vol, inonum, &ino);

    // Create the directory entry
    size_t size = sizeof(struct ext2_dirent) + strlen(file->name);

    struct ext2_dirent* dirent = kmalloc(size);

    dirent->type     = ext2_dirent_type(file->flags);
    dirent->inode    = inonum;
    dirent->name_len = strlen(file->name);
    dirent->size     = size;

    // Copy the name into the flexible array
    memcpy(dirent->name, file->name, strlen(file->name));

    struct ext2_inode pino; // Parent inode
    ext2_read_inode(vol, dir->inode, &pino);

    // Find the end of the directory
    uintptr_t offset = 0;
    uintptr_t blk    = 0;

    void* buf = kmalloc(vol->blksize);
    read_blocks(vol, buf, ext2_inode_blk(vol, &pino, blk), 1);

    while (offset + blk * vol->blksize < INOSIZE(pino))
    {
        struct ext2_dirent* entry = (struct ext2_dirent*)((uintptr_t)buf + offset);
        offset += entry->size;

        size_t expect = sizeof(struct ext2_dirent) + entry->name_len;
        expect = (expect & 0xfffffffc) + 4; // 4-byte alignment

        if (expect < entry->size)
        {
            if (entry->size - expect >= dirent->size)
            {
                size_t orig = dirent->size;
                dirent->size = entry->size - expect; // Padded to fit the rest of empty space

                // Copy the data and write the block
                memcpy((void*)entry + expect, dirent, orig);
                entry->size = expect;
                write_blocks(vol, buf, ext2_inode_blk(vol, &pino, blk), 1);

                kfree(buf);
                return;
            }
        }

        if (offset >= vol->blksize)
        {
            blk++; offset = 0;
            read_blocks(vol, buf, ext2_inode_blk(vol, &pino, blk), 1);
        }
    }

    // TODO: allocate next block and write the dent there
    kfree(buf);
}

void ext2_mkfile(struct file* dir, const char* name)
{
    struct file file;
    memset(&file, 0, sizeof(struct file));

    file.flags = FL_FILE;
    strcpy(file.name, name);

    ext2_mkentry(dir, &file);
}

void ext2_mkdir(struct file* dir, const char* name)
{
    struct file file;
    memset(&file, 0, sizeof(struct file));

    file.flags = FL_DIR;
    strcpy(file.name, name);

    ext2_mkentry(dir, &file);

    struct ext2_volume* vol = dir->device;

    struct ext2_inode ino;
    ext2_read_inode(vol, file.inode, &ino);

    // Create a directory entry to fill the block
    void* buf = kmalloc(vol->blksize);

    read_blocks(vol, buf, ext2_inode_blk(vol, &ino, 0), 1);

    // Hard link to the same directory
    size_t dotsize = sizeof(struct ext2_dirent) + 1;
    struct ext2_dirent* dot = kmalloc(dotsize);
    
    dot->inode    = file.inode;
    dot->type     = DIRENT_DIR;
    dot->name_len = strlen(".");
    dot->size     = dotsize;
    
    memcpy(dot->name, ".", 1);

    // Hard link to parent directory
    size_t parentsize = sizeof(struct ext2_dirent) + 2;
    struct ext2_dirent* parent = kmalloc(parentsize);
    
    parent->inode    = dir->inode;
    parent->type     = DIRENT_DIR;
    parent->name_len = strlen("..");
    parent->size     = vol->blksize - dotsize; // Rest of block
    
    memcpy(parent->name, "..", 2);

    memcpy(buf, dot, dot->size);
    memcpy(buf + dotsize, parent, parent->size);

    write_blocks(vol, buf, ext2_inode_blk(vol, &ino, 0), 1);
}

void ext2_mknod(struct file* dir, struct file* file)
{
    ext2_mkentry(dir, file);
}

void ext2_unlink(struct file* dir, const char* name)
{
    (void)dir; (void)name;
}

static struct file* ext2_mount(const char* dev, const void* data)
{
    (void)data;

    struct ext2_volume* vol = kmalloc(sizeof(struct ext2_volume));
    vol->device = kmalloc(sizeof(struct file));
    vfs_resolve(dev, vol->device);

    void* buf = kmalloc(512);

    vfs_read(vol->device, buf, SUPER_BLK * 1024, 512); // FIXME: here we assume that the blocks are 1024 in size

    struct sb_full* sb = (struct sb_full*)&vol->sb;
    memcpy(sb, buf, sizeof(struct ext2_sb) + sizeof(struct ext2_sbext));

    kfree(buf);

    vol->blksize = 1024u << vol->sb.log_block_sz;

    vol->group_cnt = (vol->sb.blk_cnt % vol->sb.blks_per_grp)
                   ? (vol->sb.blk_cnt / vol->sb.blks_per_grp + 1)
                   : (vol->sb.blk_cnt / vol->sb.blks_per_grp);

    void* group_buffer = kmalloc(sizeof(struct ext2_bgd) * vol->group_cnt + vol->blksize);
    read_blocks(vol, group_buffer, SUPER_BLK + 1,
                vol->group_cnt * sizeof(struct ext2_bgd) / vol->blksize + 1);

    vol->groups = kmalloc(sizeof(struct ext2_bgd) * vol->group_cnt);
    memcpy(vol->groups, group_buffer, vol->group_cnt * sizeof(struct ext2_bgd));

    kfree(group_buffer);

    struct file* file  = vfs_create_file();

    file->device       = vol;
    file->flags        = FL_DIR;
    file->inode        = 2;

    file->ops.read     = ext2_read;
    file->ops.write    = ext2_write;
    file->ops.find     = ext2_find;
    file->ops.getdents = ext2_getdents;
    file->ops.mkfile   = ext2_mkfile;
    file->ops.mkdir    = ext2_mkdir;
    file->ops.unlink   = ext2_unlink;

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