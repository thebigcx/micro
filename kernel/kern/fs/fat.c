#include <micro/fat.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <micro/heap.h>
#include <micro/fs.h>

#define FAT_ATTR_RO             0x01
#define FAT_ATTR_HIDDEN         0x02
#define FAT_ATTR_SYS            0x04
#define FAT_ATTR_VOLID          0x08
#define FAT_ATTR_DIR            0x10
#define FAT_ATTR_AR             0x20
#define FAT_ATTR_LFN            0x0f

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

    if (j == 0) *(--dst) = 0;
}

// TODO: cluster chain reading should be in a macro called FAT_READ_CLUSTERS

ssize_t fat_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct fat32_volume* vol = file->device;

    unsigned int clus = file->inode;
    unsigned int next = 0;

    uint8_t* fatbuf = kmalloc(512); // Holds File Allocation Table
    
    uint8_t* fullbuf = kmalloc(off % 512 + size + (512 - (size % 512))); // Sector-aligned
    uint8_t* fullbuf_ptr = fullbuf;
    uint64_t start = off / 512;
    uint64_t end = (off + size) / 512;

    size_t pos = 0;

    do
    {
        uint32_t fat_sector = vol->record.bpb.res_sectors + (clus * 4) / 512;
        uint32_t fat_off = (clus * 4) % 512;

        vfs_read(vol->device, fatbuf, fat_sector * 512, 512); // Probably redundant reads (same sector over and over again)
        
        uint64_t lba = clus2lba(vol, clus);

        if (pos > end) break;

        if (pos >= start)
        {
            vfs_read(vol->device, fullbuf_ptr, lba * 512, 512);
            fullbuf_ptr += 512;
        }

        pos++;

        next = *((uint32_t*)&fatbuf[fat_off]) & 0x0fffffff;
        clus = next;

    } while ((next != 0) && !((next & 0x0fffffff) >= 0x0ffffff8));

    memcpy(buf, fullbuf + (off % 512), size);

    kfree(fatbuf);
    kfree(fullbuf);

    return 0;
}

ssize_t fat_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct fat32_volume* vol = file->device;
    
    if (off + size != file->size)
    {
        // TODO: allocate more or free clusters if necessary

        file->size = off + size;
        uintptr_t parent_clus = file->parent->inode;

        // TODO: set FAT32 dirent structure size
    }

    unsigned int clus = file->inode;
    unsigned int next = 0;

    uint8_t* fatbuf = kmalloc(512); // Holds File Allocation Table
    
    uint8_t* fullbuf = kmalloc(512); // Sector-aligned
    uint64_t start = off / 512;
    uint64_t end = (off + size) / 512;

    size_t pos = 0;

    do
    {
        uint32_t fat_sector = vol->record.bpb.res_sectors + (clus * 4) / 512;
        uint32_t fat_off = (clus * 4) % 512;

        vfs_read(vol->device, fatbuf, fat_sector * 512, 512); // Probably redundant reads (same sector over and over again)
        
        uint64_t lba = clus2lba(vol, clus);

        if (pos > end) break;

        if (pos >= start)
        {
            size_t bytes = 0;
            vfs_read(vol->device, fullbuf, lba * 512, 512);

            if (pos == start)
            {
                bytes = (512 - (off % 512));
                memcpy(fullbuf + (off % 512), buf, bytes);
            }
            else if (pos == end)
            {
                bytes = ((off + size) % 512);
                memcpy(fullbuf, buf, bytes);
            }
            else
            {
                bytes = 512;
                memcpy(fullbuf, buf, bytes);
            }

            vfs_write(vol->device, fullbuf, lba * 512, 512);
            buf += bytes;
        }

        pos++;

        next = *((uint32_t*)&fatbuf[fat_off]) & 0x0fffffff;
        clus = next;

    } while ((next != 0) && !((next & 0x0fffffff) >= 0x0ffffff8));

    kfree(fatbuf);
    kfree(fullbuf);

    return 0;
}

// name: in the format file.ext (no more than 8 characters)
int fat_name_cmp(struct fat_dirent* dirent, const char* name)
{
    char* first = kmalloc(12);
    char* second = kmalloc(12);

    memcpy(first, dirent->name, 8);
    memcpy(first + 8, dirent->ext, 3);
    first[11] = 0;

    size_t len = strcspn(name, ".\0");
    size_t ext;
    if (*(name + len) != 0)
        ext = strlen(name + len + 1);
    else
        ext = 0; // No extension

    memcpy(second, name, len);
    memset(second + len, ' ', 8 - len);
    memcpy(second + 8, name + len + 1, ext);
    memset(second + 8 + ext, ' ', 3 - ext);
    second[11] = 0;

    // Convert to upper case
    for (int i = 0; i < 11; i++)
        if (second[i] >= 'a' && second[i] <= 'z')
            second[i] -= 32;

    int ret = !strcmp(first, second);

    kfree(first);
    kfree(second);

    return ret;
}

struct file* fat_find_impl(struct fat32_volume* vol, struct file* dir, unsigned int cluster, const char* name)
{
    unsigned int clus = cluster;
    unsigned int next = 0;

    uint8_t* fatbuf = kmalloc(512); // Holds File Allocation Table
    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    do
    {
        uint32_t fat_sector = vol->record.bpb.res_sectors + (clus * 4) / 512;
        uint32_t fat_off = (clus * 4) % 512;

        vfs_read(vol->device, fatbuf, fat_sector * 512, 512); // Probably redundant reads (same sector over and over again)
        
        uint64_t lba = clus2lba(vol, clus);

        vfs_read(vol->device, buf, lba * 512, 512);

        // clus
        for (unsigned int i = 0; i < 512 / sizeof(struct fat_dirent); i++)
        {
            if (buf[i].name[0] == 0 || buf[i].name[0] == 0xe5) continue;
            else
            {
                if (fat_name_cmp(&buf[i], name))
                {
                    struct file* file = kmalloc(sizeof(struct file));

                    file->parent = dir;
                    file->device = vol;
                    file->flags = (buf[i].attr & FAT_ATTR_DIR) ? FL_DIR : FL_FILE;
                    file->inode = (buf[i].cluster_u << 16) | buf[i].cluster;
                    file->size = buf[i].file_sz;

                    file->ops.read = fat_read;
                    file->ops.write = fat_write;
                    file->ops.find = fat_find;
                    file->ops.readdir = fat_readdir;

                    strcpy(file->name, name);
                    
                    kfree(buf);
                    kfree(fatbuf);

                    return file;
                }
            }
        }

        next = *((uint32_t*)&fatbuf[fat_off]) & 0x0fffffff;
        clus = next;

    } while ((next != 0) && !((next & 0x0fffffff) >= 0x0ffffff8));

    kfree(buf);
    kfree(fatbuf);

    return NULL;
}

int fat_readdir_impl(struct fat32_volume* vol, unsigned int cluster, size_t idx, struct dirent* dirent)
{
    unsigned int clus = cluster;
    unsigned int next = 0;

    uint8_t* fatbuf = kmalloc(512); // Holds File Allocation Table
    struct fat_dirent* buf = kmalloc(512); // Hold the data we care about

    do
    {
        uint32_t fat_sector = vol->record.bpb.res_sectors + (clus * 4) / 512;
        uint32_t fat_off = (clus * 4) % 512;

        vfs_read(vol->device, fatbuf, fat_sector * 512, 512); // Probably redundant reads (same sector over and over again)
        
        uint64_t lba = clus2lba(vol, clus);

        vfs_read(vol->device, buf, lba * 512, 512);

        for (unsigned int i = 0; i < 512 / sizeof(struct fat_dirent); i++)
        {
            if (buf[i].name[0] == 0 || buf[i].name[0] == 0xe5 || buf[i].attr == FAT_ATTR_LFN || buf[i].attr & FAT_ATTR_VOLID) continue;
            else
            {
                if (idx-- == 0)
                {
                    from8dot3(&buf[i], dirent->d_name);
                    return 1;
                }
            }
        }

        next = *((uint32_t*)&fatbuf[fat_off]) & 0x0fffffff;
        clus = next;

    } while ((next != 0) && !((next & 0x0fffffff) >= 0x0ffffff8));

    kfree(buf);
    kfree(fatbuf);

    return 0;
}

struct file* fat_root_find(struct file* dir, const char* name)
{
    struct fat32_volume* vol = dir->device;
    return fat_find_impl(vol, dir, vol->record.ebr.cluster_num, name);
}

int fat_root_readdir(struct file* dir, size_t idx, struct dirent* dirent)
{
    struct fat32_volume* vol = dir->device;
    return fat_readdir_impl(vol, vol->record.ebr.cluster_num, idx, dirent);
}

int fat_readdir(struct file* dir, size_t idx, struct dirent* dirent)
{
    return fat_readdir_impl(dir->device, dir->inode, idx, dirent);
}

struct file* fat_find(struct file* dir, const char* name)
{
    return fat_find_impl(dir->device, dir, dir->inode, name);
}

struct file* fat_mount(const char* dev, void* data)
{
    struct file* device = vfs_resolve(dev);

    struct fat32_volume* vol = kmalloc(sizeof(struct fat32_volume));
    vol->device = device;

    void* buf = kmalloc(512);
    vfs_read(device, buf, 0, 512);

    memcpy(&vol->record, buf, sizeof(struct fat32_record));

    kfree(buf);

    struct file* file = kmalloc(sizeof(struct file));
    file->flags = FL_MNTPT;
    file->device = vol;
    file->ops.find = fat_root_find;
    file->ops.readdir = fat_root_readdir;
    return file;
}

void fat_init()
{
    vfs_register_fs("fat", fat_mount);
}