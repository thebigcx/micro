#include <micro/fat.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <micro/heap.h>

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

// TODO
ssize_t fat_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct fat32_volume* vol = file->device;

    unsigned int clus = file->inode;
    unsigned int next = 0;

    void* fatbuf = kmalloc(512); // Holds File Allocation Table
    
    void* fullbuf = kmalloc(off % 512 + size + (512 - (size % 512))); // Sector-aligned
    void* fullbuf_ptr = fullbuf;
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

// name: in the format file.ext (no more than 8 characters)
int fat_name_cmp(struct fat_dirent* dirent, char* name)
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

    heap_check();

    kfree(first);
    kfree(second);

    return ret;
}

struct file* fat_find_impl(struct fat32_volume* vol, unsigned int cluster, const char* name)
{
    unsigned int clus = cluster;
    unsigned int next = 0;

    void* fatbuf = kmalloc(512); // Holds File Allocation Table
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

                    file->device = vol;
                    file->flags = (buf[i].attr & FAT_ATTR_DIR) ? FL_DIR : FL_FILE;
                    file->inode = (buf[i].cluster_u << 16) | buf[i].cluster;
                    file->size = buf[i].file_sz;

                    file->ops.read = fat_read;
                    file->ops.find = fat_find;

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

struct file* fat_root_find(struct file* dir, const char* name)
{
    struct fat32_volume* vol = dir->device;
    return fat_find_impl(vol, vol->record.ebr.cluster_num, name);
}

struct file* fat_find(struct file* dir, const char* name)
{
    return fat_find_impl(dir->device, dir->inode, name);
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
    return file;
}

void fat_init()
{
    vfs_register_fs("fat", fat_mount);
}