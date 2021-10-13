#include "fat.h"

#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/fs.h>
#include <micro/debug.h>
#include <micro/module.h>
#include <micro/try.h>
#include <micro/fcntl.h>
#include <micro/errno.h>

// TODO: cache the FATs in memory

static int read_sectors(struct fat32_volume* vol, void* buf, uintptr_t lba, unsigned int cnt)
{
    return vol->device->ops.read(vol->device, buf, lba * 512, cnt * 512);
}

static int write_sectors(struct fat32_volume* vol, const void* buf, uintptr_t lba, unsigned int cnt)
{
    return vol->device->ops.write(vol->device, buf, lba * 512, cnt * 512);
}

uint64_t clus2lba(struct fat32_volume* vol, unsigned int cluster)
{
    uint8_t sectors = vol->record.bpb.sectors_per_cluster;
    return vol->record.bpb.res_sectors + vol->record.bpb.fats * vol->record.ebr.sectors_per_fat + cluster * sectors - (2 * sectors);
}

void from8dot3(struct fat_dirent* dirent, char* dst)
{
    int i = 0;
    for (; i < 8; i++)
    {
        if (dirent->name[i] == ' ')
            break;
        *dst++ = dirent->name[i];
    }

    *dst++ = '.';

    int j = 0;
    for (; j < 3; j++)
    {
        if (dirent->ext[j] == ' ')
            break;
        *dst++ = dirent->ext[j];
    }

    if (j == 0) *(--dst) = 0; // We don't want a trailing '.'
    else *dst = 0;
}

void to8dot3(const char* reg_name, char* name, char* ext)
{
    size_t len = strcspn(reg_name, ".\0");
    len = min(len, 8);

    size_t extlen;
    if (*(reg_name + len) != 0)
        extlen = strlen(reg_name + len + 1);
    else
        extlen = 0; // No extension

    extlen = min(extlen, 3);

    memcpy(name, reg_name, len);
    memset(name + len, ' ', 8 - len);
    memcpy(ext, reg_name + len + 1, extlen);
    memset(ext + extlen, ' ', 3 - extlen);
}

// name: in the format file.ext 8.3 limit
int fat_name_cmp(struct fat_dirent* dirent, const char* name)
{
    char name2[8];
    char ext2[3];
    to8dot3(name, name2, ext2);

    return !strncmp((const char*)dirent->name, name2, 8) && !strncmp((const char*)dirent->ext, ext2, 3);
}

unsigned int fat_table_read(struct fat32_volume* vol, unsigned int i)
{
    uint32_t fat_sector = vol->record.bpb.res_sectors + (i * 4) / 512;
    uint32_t fat_off = (i * 4) % 512;

    uint32_t* buf = kmalloc(512);

    read_sectors(vol, buf, fat_sector, 1);

    uint32_t val = buf[fat_off / 4];

    kfree(buf);

    return val;
}

void fat_table_write(struct fat32_volume* vol, unsigned int i, unsigned int val)
{
    uint32_t fat_sector = vol->record.bpb.res_sectors + (i * 4) / 512;
    uint32_t fat_off = (i * 4) % 512;

    uint32_t* buf = kmalloc(512);

    read_sectors(vol, buf, fat_sector, 1);
    buf[fat_off / 4] = val;
    write_sectors(vol, buf, fat_sector, 1);

    kfree(buf);
}

unsigned int fat_cchain_cnt(struct fat32_volume* vol, unsigned int clus)
{
    unsigned int cnt = 0;
    do
    {
        clus = fat_table_read(vol, clus);
        cnt++;

    } while ((clus != 0) && !((clus & 0x0fffffff) >= 0x0ffffff8));

    return cnt;
}

unsigned int fat_alloc_clus(struct fat32_volume* vol)
{
    for (unsigned int i = 2; i < vol->record.bpb.sector_cnt / vol->record.bpb.sectors_per_cluster; i++)
    {
        unsigned int ent = fat_table_read(vol, i);

        if (ent == 0)
            return i;
    }

    printk("fat: could not allocate cluster\n");
    return 0;
}

ssize_t fat_read(struct file* file, void* buf, off_t off, size_t size)
{
    if ((size_t)off > file->inode->size) return 0;
    if (off + size > file->inode->size)
    {
        size = file->inode->size - off;
        if (!size) return 0;
    }

    struct fat32_volume* vol = file->inode->priv;

    unsigned int clus = file->inode->inode;
   
    uint8_t* fullbuf = kmalloc(512);
    uintptr_t ptroff = 0;

    uint64_t startblk = off / 512;
    uint64_t modoff   = off % 512;
    
    uint64_t endblk   = (off + size) / 512;
    uint64_t modend   = (off + size) % 512;

    unsigned int cnt = fat_cchain_cnt(vol, clus);

    for (unsigned int i = 0; i < cnt && i <= endblk; i++)
    {
        uint64_t lba = clus2lba(vol, clus);
        clus = fat_table_read(vol, clus);
        
        if (i < startblk) continue;

        read_sectors(vol, fullbuf, lba, 1);
        
        uint32_t start = 0;
        uint32_t bytes = 512;
        
        if (i == startblk)
        {
            start = modoff;
            bytes = 512 - start;
        }
        if (i == endblk)
            bytes = modend < size ? modend : size;
        
        memcpy((void*)((uintptr_t)buf + ptroff), fullbuf + start, bytes);
        ptroff += bytes;
    }

    kfree(fullbuf);
    return size;
}

// TODO: IMPORTANT: move directory entry parsing to a different function
void fat_resize_dirent(struct fat32_volume* vol, unsigned int clus, const char* name, size_t size)
{
    unsigned int clusters = fat_cchain_cnt(vol, clus);
    struct fat_dirent* dirents = kmalloc(512);
    for (unsigned int i = 0; i < clusters; i++)
    {
        read_sectors(vol, dirents, clus2lba(vol, clus), 1);
        for (unsigned int j = 0; j < 512 / sizeof(struct fat_dirent); j++)
        {
            if (dirents[j].name[0] == 0 || dirents[j].name[0] == 0xe5) continue;
            else
            {
                if (fat_name_cmp(&dirents[j], name))
                {
                    dirents[j].file_sz = size;
                    write_sectors(vol, dirents, clus2lba(vol, clus), 1);
                    return;
                }
            }
        }

        clus = fat_table_read(vol, clus);
    }
}

void fat_resize_file(struct inode* file, size_t size)
{
    /*if (file->size == size) return; // No need

    struct fat32_volume* vol = file->inode->priv;

    if (file->parent) // Set parent's dirent structure size
        fat_resize_dirent(vol, file->parent->inode, file->name, size);

    if (size / 512 < file->size / 512)
    {
        // Free clusters
    }
    else if (size / 512 > file->size / 512)
    {
        // Find last cluster
        unsigned int cnt = fat_cchain_cnt(vol, file->inode) - 1;
        unsigned int clus = file->inode;
        while (cnt && cnt--) clus = fat_table_read(vol, clus);

        // Allocate cluster
        for (unsigned int i = 0; i < (size / 512) - (file->size / 512); i++)
        {
            unsigned int prev = clus;
            clus = fat_alloc_clus(vol);
            fat_table_write(vol, prev, clus);
        }
        
        fat_table_write(vol, clus, 0xffffff8);
    }

    file->size = size;*/
}

int fat_lookup(struct inode* dir, const char* name, struct dentry* dentry)
{
    struct fat32_volume* vol = dir->priv;
    unsigned int clus = dir->inode;

    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    char lfns[8][LFN_LEN + 1];
    size_t lfncnt = 0;

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);

        read_sectors(vol, buf, lba, 1);

        // clus
        for (unsigned int i = 0; i < 512 / sizeof(struct fat_dirent); i++)
        {
            if (buf[i].name[0] == 0) continue; // Unused
            else if (buf[i].name[0] == 0xe5    // Unused
                  || buf[i].attr == FAT_ATTR_VOLID)
            {
                lfncnt = 0;
                continue;
            }
            else if (buf[i].attr == FAT_ATTR_LFN)
            {
                struct fat_lfn* lfn = (struct fat_lfn*)&buf[i];

                size_t j = 0;
                for (size_t k = 0; k < LFN_CHARS1; k++)
                    lfns[lfncnt][j++] = lfn->chars1[k];

                for (size_t k = 0; k < LFN_CHARS2; k++)
                    lfns[lfncnt][j++] = lfn->chars2[k];

                for (size_t k = 0; k < LFN_CHARS3; k++)
                    lfns[lfncnt][j++] = lfn->chars3[k];

                lfns[lfncnt++][LFN_LEN] = 0;
            }
            else
            {
                int cmp = 0;

                if (lfncnt)
                {
                    char lfn[128];
                    strcpy(lfn, lfns[--lfncnt]);
                    while (lfncnt--)
                        strcpy(lfn + strlen(lfn), lfns[lfncnt]);

                    cmp = !strcmp(lfn, name);
                }
                else
                    cmp = fat_name_cmp(&buf[i], name);

                if (cmp)
                {
                    struct inode* inode  = kcalloc(sizeof(struct inode));

                    strcpy(dentry->name, name);

                    inode->priv         = vol;
                    inode->mode         = (buf[i].attr & FAT_ATTR_DIR) ? S_IFDIR : S_IFREG;
                    inode->inode        = (buf[i].cluster_u << 16) | buf[i].cluster;
                    inode->size         = buf[i].file_sz;

                    inode->fops.read    = fat_read;
                    inode->fops.write   = fat_write;
                    inode->ops.lookup   = fat_lookup;
                    inode->ops.getdents = fat_getdents;
                    inode->ops.mkdir    = fat_mkdir;
                    inode->ops.unlink   = fat_unlink;

                    dentry->file = inode;
                    
                    kfree(buf);
                    return 0;
                }
            }
        }

        clus = fat_table_read(vol, clus);
    }

    kfree(buf);
    return -ENOENT;
}

ssize_t fat_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct fat32_volume* vol = file->inode->priv;
    
    // TEMP
    if (off + size > file->inode->size)
        fat_resize_file(file, off + size);

    unsigned int clus = file->inode;
    
    uint8_t* fullbuf = kmalloc(512); // Sector-aligned
    uint64_t start = off / 512;
    uint64_t end = (off + size) / 512;

    size_t pos = 0;

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);

        if (pos > end) break;

        if (pos >= start)
        {
            read_sectors(vol, fullbuf, lba, 1);

            unsigned int byte_offset = 0;
            size_t bytes = 512;

            if (pos == start)
            {
                byte_offset = off % 512;
                bytes -= off % 512;
            }
            if (pos == end)
            {
                bytes -= 512 - ((off + size) % 512);
            }
            
            memcpy(fullbuf + byte_offset, buf, bytes);
            write_sectors(vol, fullbuf, lba, 1);

            buf = (void*)((uintptr_t)buf + bytes);
        }

        pos++;

        clus = fat_table_read(vol, clus);
    }

    kfree(fullbuf);

    return 0;
}

void fat_dirent_append(struct inode* dir, struct fat_dirent* dirent)
{
    struct fat32_volume* vol = dir->priv;
    unsigned int clus = dir->inode;

    unsigned int dirents_per_clus = 512 / sizeof(struct fat_dirent);

    struct fat_dirent* buf = kmalloc(512);

    for (;;)
    {
        read_sectors(vol, buf, clus2lba(vol, clus), 1);

        for (unsigned int j = 0; j < dirents_per_clus; j++)
        {
            if (buf[j].name[0] == 0) // Unused dirent
            {
                // Read the sector, write the dirent, and flush the sector
                memcpy(&buf[j], dirent, sizeof(struct fat_dirent));

                write_sectors(vol, buf, clus2lba(vol, clus), 1);

                kfree(buf);
                return;
            }
        }

        unsigned int prev = clus;
        clus = fat_table_read(vol, clus);

        if (clus >= 0xffffff8)
        {
            // Allocate a new cluster for dirents
            clus = fat_alloc_clus(vol);
            fat_table_write(vol, prev, clus);
            fat_table_write(vol, clus, 0xffffff8);
        }
    }

    kfree(buf);
}

void fat_mkdirent(struct inode* dir, struct fat_dirent* dirent)
{
    struct fat_dirent zero;
    memset(&zero, 0, sizeof(struct fat_dirent));
    fat_dirent_append(dir, dirent);
    fat_dirent_append(dir, &zero);
}

/*void fat_mkfile(struct inode* dir, const char* name, mode_t mode, uid_t uid, gid_t gid)
{
    (void)mode; (void)uid; (void)gid;

    struct fat32_volume* vol = dir->device;

    struct fat_dirent dirent;
    memset(&dirent, 0, sizeof(struct fat_dirent));
    to8dot3(name, (char*)dirent.name, (char*)dirent.ext);

    // Give the file one cluster (TODO: maybe this isn't necessary?)
    dirent.cluster = fat_alloc_clus(vol);

    fat_mkdirent(dir, &dirent);
    fat_table_write(vol, dirent.cluster, 0xffffff8);
}*/

int fat_mkdir(struct inode* dir, const char* name, mode_t mode, uid_t uid, gid_t gid)
{
    (void)mode; (void)uid; (void)gid;
    
    struct fat32_volume* vol = dir->priv;
    
    struct fat_dirent dirent;
    memset(&dirent, 0, sizeof(struct fat_dirent));

    to8dot3(name, (char*)dirent.name, (char*)dirent.ext);

    // Give the file one cluster (TODO: maybe this isn't necessary?)
    dirent.cluster = fat_alloc_clus(vol);
    dirent.attr |= FAT_ATTR_DIR; // Directory attribute

    fat_mkdirent(dir, &dirent);
    fat_table_write(vol, dirent.cluster, 0xffffff8);
    return 0;
}

// TODO: IMPORTANT: move directory entry parsing to a different function

// TODO: should return int error code
int fat_unlink(struct inode* dir, const char* name)
{
    struct fat32_volume* vol = dir->priv;
    unsigned int clus = dir->inode;

    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        read_sectors(vol, buf, clus2lba(vol, clus), 1);

        for (unsigned int i = 0; i < 512 / sizeof(struct fat_dirent); i++)
        {
            if (   buf[i].name[0] == 0 || buf[i].name[0] == 0xe5
                || buf[i].attr == FAT_ATTR_LFN || buf[i].attr & FAT_ATTR_VOLID) continue;
            else
            {
                if (fat_name_cmp(&buf[i], name))
                {
                    struct fat_dirent* buf = kmalloc(512);
                    read_sectors(vol, buf, clus2lba(vol, clus), 1);
                    

                    // TODO: delete the data

                    memset(&buf[i], 0, sizeof(struct fat_dirent));
                    buf[i].name[0] = 0xe5; // Bit of a hack - mark the dirent as unused (no data is actually freed)

                    write_sectors(vol, buf, clus2lba(vol, clus), 1);

                    kfree(buf);
                    return 0;
                }
            }
        }

        clus = fat_table_read(vol, clus);
    }

    kfree(buf);
    return -ENOENT;
}

unsigned int todenttype(uint8_t attr)
{
    return attr & FAT_ATTR_DIR ? DT_DIR : DT_REG;
}

#define DENTS_PER_SECT 512 / sizeof(struct fat_dirent)

ssize_t fat_getdents(struct inode* dir, off_t off, size_t size, struct dirent* dirp)
{
    struct fat32_volume* vol = dir->priv;
    unsigned int clus = dir->inode;

    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    ssize_t bytes = 0;
    size_t dirp_idx = 0;

    char lfns[8][LFN_LEN + 1];
    size_t lfncnt = 0;

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);
        read_sectors(vol, buf, lba, 1);

        for (unsigned int j = 0; j < DENTS_PER_SECT; j++)
        {
            if (buf[j].name[0] == 0) continue; // Unused
            else if (buf[j].name[0] == 0xe5    // Unused
                  || buf[j].attr == FAT_ATTR_VOLID)
            {
                lfncnt = 0;
                continue;
            }
            else if (buf[j].attr == FAT_ATTR_LFN)
            {
                struct fat_lfn* lfn = (struct fat_lfn*)&buf[j];

                size_t k = 0;
                for (size_t l = 0; l < LFN_CHARS1; l++)
                    lfns[lfncnt][k++] = lfn->chars1[l];

                for (size_t l = 0; l < LFN_CHARS2; l++)
                    lfns[lfncnt][k++] = lfn->chars2[l];

                for (size_t l = 0; l < LFN_CHARS3; l++)
                    lfns[lfncnt][k++] = lfn->chars3[l];

                lfns[lfncnt++][LFN_LEN] = 0;
            }
            else
            {
                if (bytes + sizeof(struct dirent) >= size)
                {
                    kfree(buf);
                    return bytes;
                }

                if (off--)
                {
                    lfncnt = 0; continue;
                }

                bytes += sizeof(struct dirent);

                dirp[dirp_idx].d_ino    = (buf[j].cluster_u << 32) | buf[j].cluster;
                dirp[dirp_idx].d_off    = sizeof(struct dirent) * (dirp_idx + 1);
                dirp[dirp_idx].d_reclen = sizeof(struct dirent);
                dirp[dirp_idx].d_type   = todenttype(buf[j].attr);

                char* dirpname = dirp[dirp_idx].d_name;
                if (lfncnt)
                {
                    strcpy(dirpname, lfns[--lfncnt]);
                    while (lfncnt--)
                        strcpy(dirpname + strlen(dirpname), lfns[lfncnt]);
                }
                else
                    from8dot3(&buf[j], dirpname);

                dirp_idx++;

                lfncnt = 0;
            }
        }

        clus = fat_table_read(vol, clus);
    }

    kfree(buf);
    return bytes;
}

int fat_mount(const char* dev, const void* data, struct inode* fsroot)
{
    (void)data;

    struct file* device = kcalloc(sizeof(struct file));
    TRY(vfs_open(dev, device, O_RDWR));

    struct fat32_volume* vol = kmalloc(sizeof(struct fat32_volume));
    vol->device = device;

    void* buf = kmalloc(512);
    read_sectors(vol, buf, 0, 1);

    memcpy(&vol->record, buf, sizeof(struct fat32_record));

    kfree(buf);

    fsroot->mode         = 0555 | S_IFDIR;
    fsroot->priv         = vol;
    fsroot->inode        = vol->record.ebr.cluster_num;
    fsroot->ops.lookup   = fat_lookup;
    fsroot->ops.getdents = fat_getdents;
    fsroot->ops.mkdir    = fat_mkdir;
    fsroot->ops.unlink   = fat_unlink;

    printk("mounted FAT32 filesystem on %s\n", dev);

    return 0;
}

void fat_init()
{
    printk("loaded FAT32 driver\n");
    vfs_register_fs("fat", fat_mount);
}

void fat_fini()
{
    printk("finalizing FAT32 driver\n");
}

struct modmeta meta =
{
    .init = fat_init,
    .fini = fat_fini,
    .name = "fat"
};
