#include "fat.h"

#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/fs.h>
#include <micro/debug.h>
#include <micro/module.h>

#define FAT_ATTR_RO             0x01
#define FAT_ATTR_HIDDEN         0x02
#define FAT_ATTR_SYS            0x04
#define FAT_ATTR_VOLID          0x08
#define FAT_ATTR_DIR            0x10
#define FAT_ATTR_AR             0x20
#define FAT_ATTR_LFN            0x0f

// TODO: cache the FATs in memory

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
        else if (dirent->name[i] >= 'A' && dirent->name[i] <= 'Z')
            *dst++ = dirent->name[i] + 32; // convert to lower case
        else
            *dst++ = dirent->name[i];
    }

    *dst++ = '.';

    int j = 0;
    for (; j < 3; j++)
    {
        if (dirent->ext[j] == ' ')
            break;
        else if (dirent->ext[j] >= 'A' && dirent->ext[j] <= 'Z')
            *dst++ = dirent->ext[j] + 32; // convert to lower case
        else
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

    // Convert to upper case
    for (int i = 0; i < 8; i++)
        if (name[i] >= 'a' && name[i] <= 'z')
            name[i] -= 32;

    for (int i = 0; i < 3; i++)
        if (ext[i] >= 'a' && ext[i] <= 'z')
            ext[i] -= 32;
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

    vfs_read(vol->device, buf, fat_sector * 512, 512);

    uint32_t val = buf[fat_off / 4];

    kfree(buf);

    return val;
}

void fat_table_write(struct fat32_volume* vol, unsigned int i, unsigned int val)
{
    uint32_t fat_sector = vol->record.bpb.res_sectors + (i * 4) / 512;
    uint32_t fat_off = (i * 4) % 512;

    uint32_t* buf = kmalloc(512);

    vfs_read(vol->device, buf, fat_sector * 512, 512);
    buf[fat_off / 4] = val;
    vfs_write(vol->device, buf, fat_sector * 512, 512);

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
    struct fat32_volume* vol = file->device;

    unsigned int clus = file->inode;
    
    uint8_t* fullbuf = kmalloc(off % 512 + size + (512 - (size % 512))); // Sector-aligned
    uint8_t* fullbuf_ptr = fullbuf;
    uint64_t start = off / 512;
    uint64_t end = (off + size) / 512;

    // TODO: use 'i' rather than this variable
    size_t pos = 0;

    unsigned int cnt = fat_cchain_cnt(vol, clus);

    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);

        if (pos > end) break;
        if (pos >= start)
        {
            vfs_read(vol->device, fullbuf_ptr, lba * 512, 512);
            fullbuf_ptr += 512;
        }

        pos++;
        clus = fat_table_read(vol, clus);
    }

    memcpy(buf, fullbuf + (off % 512), size);

    kfree(fullbuf);

    return 0;
}

// TODO: IMPORTANT: move directory entry parsing to a different function
void fat_resize_dirent(struct fat32_volume* vol, unsigned int clus, const char* name, size_t size)
{
    unsigned int clusters = fat_cchain_cnt(vol, clus);
    struct fat_dirent* dirents = kmalloc(512);
    for (unsigned int i = 0; i < clusters; i++)
    {
        vfs_read(vol->device, dirents, clus2lba(vol, clus) * 512, 512);
        for (unsigned int j = 0; j < 512 / sizeof(struct fat_dirent); j++)
        {
            if (dirents[j].name[0] == 0 || dirents[j].name[0] == 0xe5) continue;
            else
            {
                if (fat_name_cmp(&dirents[j], name))
                {
                    dirents[j].file_sz = size;
                    vfs_write(vol->device, dirents, clus2lba(vol, clus) * 512, 512);
                    return;
                }
            }
        }

        clus = fat_table_read(vol, clus);
    }
}

void fat_resize_file(struct file* file, size_t size)
{
    if (file->size == size) return; // No need

    struct fat32_volume* vol = file->device;

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

    file->size = size;
}

ssize_t fat_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct fat32_volume* vol = file->device;
    
    // TEMP
    if (off + size > file->size)
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
            vfs_read(vol->device, fullbuf, lba * 512, 512);

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
            vfs_write(vol->device, fullbuf, lba * 512, 512);

            buf = (void*)((uintptr_t)buf + bytes);
        }

        pos++;

        clus = fat_table_read(vol, clus);
    }

    kfree(fullbuf);

    return 0;
}

void fat_dirent_append(struct file* dir, struct fat_dirent* dirent)
{
    struct fat32_volume* vol = dir->device;
    unsigned int clus = dir->inode;

    unsigned int dirents_per_clus = 512 / sizeof(struct fat_dirent);

    struct fat_dirent* buf = kmalloc(512);

    for (;;)
    {
        vfs_read(vol->device, buf, clus2lba(vol, clus) * 512, 512);

        for (unsigned int j = 0; j < dirents_per_clus; j++)
        {
            if (buf[j].name[0] == 0) // Unused dirent
            {
                // Read the sector, write the dirent, and flush the sector
                memcpy(&buf[j], dirent, sizeof(struct fat_dirent));

                vfs_write(vol->device, buf, clus2lba(vol, clus) * 512, 512);

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

void fat_mkdirent(struct file* dir, struct fat_dirent* dirent)
{
    struct fat_dirent zero;
    memset(&zero, 0, sizeof(struct fat_dirent));
    fat_dirent_append(dir, dirent);
    fat_dirent_append(dir, &zero);
}

void fat_mkfile(struct file* dir, const char* name)
{
    struct fat32_volume* vol = dir->device;

    struct fat_dirent dirent;
    memset(&dirent, 0, sizeof(struct fat_dirent));
    to8dot3(name, (char*)dirent.name, (char*)dirent.ext);

    // Give the file one cluster (TODO: maybe this isn't necessary?)
    dirent.cluster = fat_alloc_clus(vol);

    fat_mkdirent(dir, &dirent);
    fat_table_write(vol, dirent.cluster, 0xffffff8);
}

void fat_mkdir(struct file* dir, const char* name)
{
    struct fat32_volume* vol = dir->device;
    
    struct fat_dirent dirent;
    memset(&dirent, 0, sizeof(struct fat_dirent));

    to8dot3(name, (char*)dirent.name, (char*)dirent.ext);

    // Give the file one cluster (TODO: maybe this isn't necessary?)
    dirent.cluster = fat_alloc_clus(vol);
    dirent.attr |= FAT_ATTR_DIR; // Directory attribute

    fat_mkdirent(dir, &dirent);
    fat_table_write(vol, dirent.cluster, 0xffffff8);
}

// TODO: IMPORTANT: move directory entry parsing to a different function

void fat_rm(struct file* dir, const char* name)
{
    struct fat32_volume* vol = dir->device;
    unsigned int clus = dir->inode;

    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);

        vfs_read(vol->device, buf, lba * 512, 512);

        for (unsigned int i = 0; i < 512 / sizeof(struct fat_dirent); i++)
        {
            if (   buf[i].name[0] == 0 || buf[i].name[0] == 0xe5
                || buf[i].attr == FAT_ATTR_LFN || buf[i].attr & FAT_ATTR_VOLID) continue;
            else
            {
                if (fat_name_cmp(&buf[i], name))
                {
                    struct fat_dirent* buf = kmalloc(512);
                    vfs_read(vol->device, buf, clus2lba(vol, clus) * 512, 512);

                    // TODO: delete the data

                    memset(&buf[i], 0, sizeof(struct fat_dirent));

                    vfs_write(vol->device, buf, clus2lba(vol, clus) * 512, 512);

                    kfree(buf);
                    return;
                }
            }
        }

        clus = fat_table_read(vol, clus);
    }

    kfree(buf);
}

/*int fat_readdir(struct file* dir, size_t idx, struct dirent* dirent)
{
    struct fat32_volume* vol = dir->device;
    unsigned int clus = dir->inode;

    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);

        vfs_read(vol->device, buf, lba * 512, 512);

        for (unsigned int i = 0; i < 512 / sizeof(struct fat_dirent); i++)
        {
            if (   buf[i].name[0] == 0 || buf[i].name[0] == 0xe5
                || buf[i].attr == FAT_ATTR_LFN || buf[i].attr & FAT_ATTR_VOLID) continue;
            else
            {
                if (idx-- == 0)
                {
                    from8dot3(&buf[i], dirent->d_name);
                    return 1;
                }
            }
        }

        clus = fat_table_read(vol, clus);
    }

    kfree(buf);

    return 0;
}*/

#define DENTS_PER_SECT 512 / sizeof(struct fat_dirent)

ssize_t fat_getdents(struct file* dir, off_t off, size_t size, struct dirent* dirp)
{
    struct fat32_volume* vol = dir->device;
    unsigned int clus = dir->inode;

    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    ssize_t bytes = 0;
    size_t dirp_idx = 0;
    size_t dirent_idx = 0;

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);

        vfs_read(vol->device, buf, lba * 512, 512);

        for (unsigned int j = 0; j < DENTS_PER_SECT; j++)
        {
            if (   buf[j].name[0] == 0 || buf[j].name[0] == 0xe5
                || buf[j].attr == FAT_ATTR_LFN || buf[j].attr & FAT_ATTR_VOLID) continue;
            else
            {
                if (dirent_idx >= off + size)
                {
                    kfree(buf);
                    return bytes;
                }

                if (dirent_idx++ < off) continue;

                bytes += sizeof(struct fat_dirent);
                from8dot3(&buf[j], dirp[dirp_idx].d_name);
                dirp_idx++;
            }
        }

        clus = fat_table_read(vol, clus);
    }

    kfree(buf);
    return bytes;
}

struct file* fat_find(struct file* dir, const char* name)
{
    struct fat32_volume* vol = dir->device;
    unsigned int clus = dir->inode;

    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    unsigned int cnt = fat_cchain_cnt(vol, clus);
    for (unsigned int i = 0; i < cnt; i++)
    {
        uint64_t lba = clus2lba(vol, clus);

        vfs_read(vol->device, buf, lba * 512, 512);

        // clus
        for (unsigned int i = 0; i < 512 / sizeof(struct fat_dirent); i++)
        {
            if (   buf[i].name[0] == 0 || buf[i].name[0] == 0xe5
                || buf[i].attr == FAT_ATTR_LFN || buf[i].attr & FAT_ATTR_VOLID) continue;
            else
            {
                if (fat_name_cmp(&buf[i], name))
                {
                    struct file* file  = vfs_create_file();

                    file->parent       = dir;
                    file->device       = vol;
                    file->flags        = (buf[i].attr & FAT_ATTR_DIR) ? FL_DIR : FL_FILE;
                    file->inode        = (buf[i].cluster_u << 16) | buf[i].cluster;
                    file->size         = buf[i].file_sz;

                    file->ops.read     = fat_read;
                    file->ops.write    = fat_write;
                    file->ops.find     = fat_find;
                    //file->ops.readdir = fat_readdir;
                    file->ops.getdents = fat_getdents;
                    file->ops.mkfile   = fat_mkfile;
                    file->ops.mkdir    = fat_mkdir;
                    file->ops.rm       = fat_rm;

                    strcpy(file->name, name);
                    
                    kfree(buf);

                    return file;
                }
            }
        }

        clus = fat_table_read(vol, clus);
    }

    kfree(buf);

    return NULL;
}

struct file* fat_mount(const char* dev, void* data)
{
    struct file* device = kmalloc(sizeof(struct file));
    vfs_resolve(dev, device);

    struct fat32_volume* vol = kmalloc(sizeof(struct fat32_volume));
    vol->device = device;

    void* buf = kmalloc(512);
    vfs_read(device, buf, 0, 512);

    memcpy(&vol->record, buf, sizeof(struct fat32_record));

    kfree(buf);

    struct file* file = vfs_create_file();
    memset(file, 0, sizeof(struct file));

    file->flags        = FL_DIR;
    file->device       = vol;
    file->inode        = vol->record.ebr.cluster_num;
    file->ops.find     = fat_find;
    //file->ops.readdir = fat_readdir;
    file->ops.getdents = fat_getdents;
    file->ops.mkfile   = fat_mkfile;
    file->ops.mkdir    = fat_mkdir;
    file->ops.rm       = fat_rm;

    return file;
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