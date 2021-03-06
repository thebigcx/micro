#include "ext2.h"

#include <micro/heap.h>
#include <micro/stdlib.h>
#include <micro/module.h>
#include <micro/debug.h>
#include <micro/errno.h>
#include <micro/fcntl.h>
#include <arch/cmos.h>
#include <micro/try.h>

#define SUPER_BLK 1
#define BGDS_BLK  2

#define INOSIZE(ino) (((size_t)((ino).size_u) << 32) | (ino).size)

static ssize_t read_blocks(struct ext2_volume* vol, void* buf,
                           uintptr_t blk, size_t cnt)
{
    return vfs_pread(vol->device, buf, vol->blksize * cnt, blk * vol->blksize);
}

static ssize_t write_blocks(struct ext2_volume* vol, const void* buf,
                            uintptr_t blk, size_t cnt)
{
    return vfs_pwrite(vol->device, buf, vol->blksize * cnt, blk * vol->blksize);
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

const struct file_ops ext2_fops =
{
    .read = ext2_read,
    .write = ext2_write,
    .chown = ext2_chown,
    .chmod = ext2_chmod
};

const struct inode_ops ext2_iops =
{
    .getdents = ext2_getdents,
    .mkdir    = ext2_mkdir,
    .mknod    = ext2_mknod,
    .unlink   = ext2_unlink,
    .readlink = ext2_readlink,
    .symlink  = ext2_symlink,
    .link     = ext2_link,
    .lookup   = ext2_lookup
};

static void inode2file(struct ext2_volume* vol, unsigned int inonum,
                       struct ext2_inode* ino, struct inode* file)
{
    file->ops     = ext2_iops;
    file->fops    = ext2_fops;

    file->inode   = inonum;
    file->size    = INOSIZE(*ino);
    file->priv    = vol;
    file->nlink   = ino->nlink;

    file->atime   = ino->atime;
    file->ctime   = ino->ctime;
    file->mtime   = ino->mtime;

    file->mode    = ino->mode;

    file->uid     = ino->uid;
    file->gid     = ino->gid;
    file->blksize = vol->blksize;
    file->blocks  = ino->sectors * (vol->blksize / 512);
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
        return ino->blocks[i];
    }
    else if (i < dind) // Singly indirect
    {
        uint32_t* buf = kmalloc(vol->blksize);

        read_blocks(vol, buf, ino->blocks[INO_SIND], 1);
        uint32_t ret = buf[i - INO_SIND];
        kfree(buf);

        return ret;
    }
    else if (i < tind) // Doubly indirect
    {
        uint32_t* blk_ptrs = kmalloc(vol->blksize);
        uint32_t* buf = kmalloc(vol->blksize);

        read_blocks(vol, blk_ptrs, ino->blocks[INO_DIND], 1); // First indirect
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
        
        read_blocks(vol, sind_ptrs, ino->blocks[INO_TIND],       1); // First indirect
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
        ino->blocks[i] = blk;
        ino->sectors += vol->blksize / 512;
    }
    else if (i < dind)
    {
        uint32_t* buf = kmalloc(vol->blksize);

        read_blocks(vol, buf, ino->blocks[INO_SIND], 1);
        buf[i - INO_SIND] = blk;
        write_blocks(vol, buf, ino->blocks[INO_SIND], 1);

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
    switch (type & S_IFMT)
    {
        case S_IFREG:  return DIRENT_FILE;
        case S_IFDIR:  return DIRENT_DIR;
        case S_IFCHR:  return DIRENT_CHARDEV;
        case S_IFBLK:  return DIRENT_BLOCKDEV;
        case S_IFIFO:  return DIRENT_FIFO;
        case S_IFSOCK: return DIRENT_SOCKET;
        case S_IFLNK:  return DIRENT_SYMLINK;
    }

    printk("ext2: unknown file type %d\n", type);
    return DIRENT_UNK;
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
        return i * vol->sb.blks_per_grp + j + 1;
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

static int todenttype(int type)
{
    switch (type)
    {
        case DIRENT_UNK:      return DT_UNKNOWN;
        case DIRENT_FILE:     return DT_REG;
        case DIRENT_DIR:      return DT_DIR;
        case DIRENT_CHARDEV:  return DT_CHR;
        case DIRENT_BLOCKDEV: return DT_BLK;
        case DIRENT_FIFO:     return DT_FIFO;
        case DIRENT_SOCKET:   return DT_SOCK;
        case DIRENT_SYMLINK:  return DT_LNK;
    }
    
    return DT_UNKNOWN;
}

ssize_t ext2_getdents(struct inode* dir, off_t off, size_t n, struct dirent* dirp)
{
    struct ext2_volume* vol = dir->priv;

    struct ext2_inode ino;
    ext2_read_inode(vol, dir->inode, &ino);

    // TODO: maybe this is unnecessary
    size_t dentidx = 0;
    ssize_t bytes = 0;

    uintptr_t offset = 0;
    uintptr_t blk    = 0;

    void* buf = kmalloc(vol->blksize);
    read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);

    while (offset + blk * vol->blksize < ino.sectors * 512)
    {
        struct ext2_dirent* dirent = (struct ext2_dirent*)((uintptr_t)buf + offset);
        offset += dirent->size;

        if (dirent->inode == 0 || off--) continue;

        if (bytes + sizeof(struct dirent) >= n)
        {
            kfree(buf);
            return bytes;
        }

        // TODO: struct dirent should hold a flexible array for the name
        dirp[dentidx].d_ino    = dirent->inode;
        dirp[dentidx].d_off    = sizeof(struct dirent) * (dentidx + 1);
        dirp[dentidx].d_reclen = sizeof(struct dirent);
        dirp[dentidx].d_type   = todenttype(dirent->type);

        strncpy(dirp[dentidx].d_name, dirent->name, dirent->name_len);

        dentidx++;
        bytes += sizeof(struct dirent);

        if (offset >= vol->blksize)
        {
            blk++; offset = 0;
            read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);
        }
    }

    kfree(buf);
    return bytes;
}

void ext2_resize(struct inode* file, size_t size)
{
    struct ext2_volume* vol = file->priv;

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode, &ino);

    // Get number of blocks needed to resize
    size_t diff = size - file->size;
    size_t cnt = diff % vol->blksize
               ? diff / vol->blksize + 1
               : diff / vol->blksize;

    while (cnt--)
    {
        // Allocate blocks on the end
        uint32_t i = ino.sectors / (vol->blksize / 512);
        ext2_set_inode_blk(vol, &ino, i, ext2_alloc_blk(vol));
    }

    ino.size   = size & 0xffffffff;
    ino.size_u = size >> 32;

    ext2_write_inode(vol, file->inode, &ino);

    file->size = size;
}

ssize_t ext2_read(struct file* file, void* buf, off_t off, size_t size)
{
    if ((size_t)off > file->inode->size) return 0;
    if (off + size > file->inode->size)
    {
        size = file->inode->size - off;
        if (!size) return 0;
    }

    struct ext2_volume* vol = file->inode->priv;

    uint32_t startblk =  off         / vol->blksize; // Start block
    uint32_t modoff   =  off         % vol->blksize; // Byte offset of start block

    uint32_t endblk   = (size + off) / vol->blksize; // End block
    uint32_t modend   = (size + off) % vol->blksize; // Byte offset of end block

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode->inode, &ino);

    uint8_t* fullbuf = kmalloc(vol->blksize);
    
    uint64_t ptroff = 0;
    for (uint32_t i = startblk; i <= endblk; i++)
    {
        read_blocks(vol, fullbuf, ext2_inode_blk(vol, &ino, i), 1);

        uint32_t start = 0;
        uint32_t bytes = vol->blksize;

        if (i == startblk)
        {
            start = modoff;
            bytes = vol->blksize - start;
        }
        if (i == endblk)
        {
            bytes = modend < size ? modend : size;
        }

        memcpy((void*)((uintptr_t)buf + ptroff), fullbuf + start, bytes);

        ptroff += bytes;
    }

    kfree(fullbuf);
    return size;
}

ssize_t ext2_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct ext2_volume* vol = file->inode->priv;

    if (file->inode->size < off + size)
    {
        ext2_resize(file->inode, off + size);
    }

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode->inode, &ino);

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
            wsize -= start;
        }
        if (i == endblk)
            wsize -= vol->blksize - modend;

        memcpy(fullbuf + start, (void*)((uintptr_t)buf + ptroff), wsize);

        write_blocks(vol, fullbuf, ext2_inode_blk(vol, &ino, i), 1);

        ptroff += wsize;
    }

    kfree(fullbuf);
    return size;
}

int ext2_lookup(struct inode* dir, const char* name, struct dentry* dentry)
{
    struct ext2_volume* vol = dir->priv;

    struct ext2_inode ino;
    ext2_read_inode(vol, dir->inode, &ino);

    uintptr_t offset = 0;
    uintptr_t blk    = 0;

    void* buf = kmalloc(vol->blksize);
    read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);

    while (offset + blk * vol->blksize < ino.sectors * 512)
    {
        struct ext2_dirent* dirent = (struct ext2_dirent*)((uintptr_t)buf + offset);
        offset += dirent->size;

        if (strlen(name) == dirent->name_len && !strncmp(name, dirent->name, dirent->name_len))
        {
            strcpy(dentry->name, name);
            
            struct ext2_inode cino;
            ext2_read_inode(vol, dirent->inode, &cino);

            dentry->file = kcalloc(sizeof(struct inode));
            inode2file(vol, dirent->inode, &cino, dentry->file);
            return 0;
        }

        if (offset >= vol->blksize)
        {
            blk++; offset = 0;
            read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);
        }
    }

    kfree(buf);

    return -ENOENT;
}

int ext2_set_atime(struct inode* ino, time_t atime)
{
    ino->atime = atime;

    struct ext2_inode eino;
    ext2_read_inode(ino->priv, ino->inode, &eino);
    eino.atime = atime;
    ext2_write_inode(ino->priv, ino->inode, &eino);

    return 0;
}

int ext2_set_mtime(struct inode* ino, time_t mtime)
{
    ino->atime = mtime;

    struct ext2_inode eino;
    ext2_read_inode(ino->priv, ino->inode, &eino);
    eino.mtime = mtime;
    ext2_write_inode(ino->priv, ino->inode, &eino);

    return 0;
}

void ext2_init_inode(struct ext2_volume* vol, struct ext2_inode* ino, struct inode* file)
{
    (void)vol;
    
    memset(ino, 0, sizeof(struct ext2_inode));

    ino->mode   = file->mode;
    ino->uid    = file->uid;
    ino->gid    = file->gid;

    ino->size    = file->size;
    ino->sectors = 0;
    ino->nlink   = 1;
    ino->ctime   = time_getepoch();
    ino->atime   = ino->ctime;
    ino->mtime   = ino->ctime;
}

static void ext2_append_dirent(struct inode* dir, struct ext2_dirent* dirent)
{
    struct ext2_volume* vol = dir->priv;

    struct ext2_inode pino; // Parent inode
    ext2_read_inode(vol, dir->inode, &pino);

    // Find the end of the directory
    uintptr_t offset = 0;
    uintptr_t blk    = 0;

    void* buf = kmalloc(vol->blksize);
    read_blocks(vol, buf, ext2_inode_blk(vol, &pino, blk), 1);

    while (offset + blk * vol->blksize < pino.sectors * 512)
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
    
    uint32_t i = pino.sectors / (vol->blksize / 512);
    ext2_set_inode_blk(vol, &pino, i, ext2_alloc_blk(vol));

    memset(buf, 0, vol->blksize);
    memcpy(buf, dirent, dirent->size);
    ((struct ext2_dirent*)buf)->size = vol->blksize;
    write_blocks(vol, buf, ext2_inode_blk(vol, &pino, blk), 1);

    ext2_write_inode(vol, dir->inode, &pino); 

    kfree(buf);
}

// TODO: struct dentry* instead of this garbage
void ext2_mkentry(struct inode* dir, struct inode* file, const char* name)
{
    struct ext2_volume* vol = dir->priv;

    uint32_t inonum = ext2_alloc_inode(vol);

    file->inode = inonum;

    // Initialize the inode and write to disk
    struct ext2_inode ino;
    ext2_read_inode (vol, inonum, &ino);
    ext2_init_inode (vol, &ino, file);
    ext2_write_inode(vol, inonum, &ino);

    // Create the directory entry
    size_t size = sizeof(struct ext2_dirent) + strlen(name);

    struct ext2_dirent* dirent = kmalloc(size);

    dirent->type     = ext2_dirent_type(file->mode);
    dirent->inode    = inonum;
    dirent->name_len = strlen(name);
    dirent->size     = size;

    // Copy the name into the flexible array
    memcpy(dirent->name, name, strlen(name));

    ext2_append_dirent(dir, dirent);
}

int ext2_mkdir(struct inode* dir, const char* name, mode_t mode, uid_t uid, gid_t gid)
{
    struct inode file;
    memset(&file, 0, sizeof(struct inode));

    file.uid   = uid;
    file.gid   = gid;
    file.mode  = (mode & S_PERMS) | S_IFDIR;
    file.size  = 1024;

    ext2_mkentry(dir, &file, name);

    struct ext2_volume* vol = dir->priv;

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

    return 0;
}

int ext2_mknod(struct inode* dir, const char* name, mode_t mode, dev_t dev, uid_t uid, gid_t gid)
{
    struct inode file;
    memset(&file, 0, sizeof(struct inode));
    
    file.mode  = mode;
    file.uid   = uid;
    file.gid   = gid;
    
    if (dev)
        file.rdev = dev;

    ext2_mkentry(dir, &file, name);

    return 0;
}

// TODO: error code
int ext2_unlink(struct inode* dir, const char* name)
{
    struct ext2_volume* vol = dir->priv;

    struct ext2_inode ino;
    ext2_read_inode(vol, dir->inode, &ino);

    uintptr_t offset = 0;
    uintptr_t blk    = 0;

    void* buf = kmalloc(vol->blksize);
    read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);

    while (offset + blk * vol->blksize < ino.sectors * 512)
    {
        struct ext2_dirent* entry = (struct ext2_dirent*)((uintptr_t)buf + offset);
        offset += entry->size;

        if (strlen(name) == entry->name_len && !strncmp(name, entry->name, entry->name_len))
        {
            // Zero the dirent, leaving 'size' untouched
            size_t size = entry->size;
            memset(entry, 0, size);
            entry->size = size;

            write_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);
            kfree(buf);
            return 0;
        }

        if (offset >= vol->blksize)
        {
            blk++; offset = 0;
            read_blocks(vol, buf, ext2_inode_blk(vol, &ino, blk), 1);
        }
    }

    kfree(buf);
    return -ENOENT;
}

int ext2_chmod(struct file* file, mode_t mode)
{
    struct ext2_volume* vol = file->inode->priv;

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode->inode, &ino);
    ino.mode = (ino.mode & S_IFMT) | (mode & S_PERMS);
    ext2_write_inode(vol, file->inode->inode, &ino);

    return 0;
}

int ext2_chown(struct file* file, uid_t uid, gid_t gid)
{
    struct ext2_volume* vol = file->inode->priv;

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode->inode, &ino);
    
    if (uid != -1) ino.uid = uid;
    if (gid != -1) ino.gid = gid;

    ext2_write_inode(vol, file->inode->inode, &ino);

    return 0;
}

int ext2_readlink(struct inode* file, char* buf, size_t n)
{
    if (!S_ISLNK(file->mode)) return -EINVAL;

    struct ext2_inode ino;
    ext2_read_inode(file->priv, file->inode, &ino);

    n = INOSIZE(ino) < n ? INOSIZE(ino) : n;

    char* link = (char*)ino.blocks;
    memcpy(buf, link, n);

    return n;
}

int ext2_symlink(struct inode* file, const char* link)
{
    struct ext2_volume* vol = file->priv;

    struct ext2_inode ino;
    ext2_read_inode(vol, file->inode, &ino);
    
    memcpy(ino.blocks, link, strlen(link));
    ino.size = strlen(link);
    
    ext2_write_inode(vol, file->inode, &ino);

    file->size = INOSIZE(ino);

    return 0;
}

int ext2_link(struct inode* old, const char* name, struct inode* dir)
{
    struct ext2_volume* vol = old->priv;
    (void)vol;

    size_t size = sizeof(struct ext2_dirent) + strlen(name);

    struct ext2_dirent* dirent = kmalloc(size);
    
    dirent->inode = old->inode;
    dirent->name_len = strlen(name);
    dirent->size = size;
    dirent->type = ext2_dirent_type(old->mode);
    
    strncpy(dirent->name, name, strlen(name));

    ext2_append_dirent(dir, dirent);

    // TODO: increment inode 'links' field

    return 0;
}

static int ext2_mount(const char* dev, const void* data, struct inode* fsroot)
{
    (void)data;

    struct ext2_volume* vol = kmalloc(sizeof(struct ext2_volume));
    vol->device = kmalloc(sizeof(struct file));
   
    TRY(vfs_open(dev, vol->device, O_RDWR));

    void* buf = kmalloc(512);

    vfs_pread(vol->device, buf, 512, SUPER_BLK * 1024); // FIXME: here we assume that the blocks are 1024 in size

    struct sb_full* sb = (struct sb_full*)&vol->sb;
    memcpy(sb, buf, sizeof(struct ext2_sb) + sizeof(struct ext2_sbext));

    if (vol->sb.ext2_sig != 0xef53) return -EINVAL;

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

    struct ext2_inode ino;
    ext2_read_inode(vol, 2, &ino);

    printk("mounted Ext2 filesystem on %s\n", dev);
    inode2file(vol, 2, &ino, fsroot);
    return 0;
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
