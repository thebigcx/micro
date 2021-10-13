#pragma once

#include <micro/types.h>
#include <micro/platform.h>

// BIOS parameter block
struct PACKED fat_bpb
{
    uint8_t  jmp[3];                // Jump over BPB (boot code)
    int8_t   oem_id[8];             // OEM identifier
    uint16_t sector_sz;             // Bytes per sector
    uint8_t  sectors_per_cluster;   // Sectors per cluster
    uint16_t res_sectors;           // Number of reserved sectors
    uint8_t  fats;                  // Number of File Allocation Tables
    uint16_t dir_entries;           // Number of directory entries
    uint16_t sector_cnt;            // Total number of sectors in volume
    uint8_t  media_desc_type;       // Media descriptor type
    uint16_t sectors_per_fat;       // Number of sectors per FAT (FAT12/FAT16 only)
    uint16_t sectors_per_track;     // Number of sectors per track
    uint16_t head_cnt;              // Number of heads on media
    uint32_t hidden_sectors;        // Number of hidden sectors
    uint32_t large_sector_cnt;      // Large sector count
};

// FAT32 extended boot record
struct PACKED fat_ebr
{
    uint32_t sectors_per_fat;       // Size of FAT in sectors
    uint16_t flags;                 // Flags
    uint16_t fat_vs;                // FAT version
    uint32_t cluster_num;           // Cluster number of root directory
    uint16_t fsinfo_sector;         // The sector number of the FSInfo structure
    uint16_t backup_bs;             // Backup boot sector
    uint8_t  res0[12];              // Reserved
    uint8_t  drive_num;             // Drive number
    uint8_t  res1;                  // Reserved
    uint8_t  sig;                   // Signature
    uint32_t volume_id;             // Volume ID serial number
    int8_t   volume_label[11];      // Volume label string
    int8_t   sys_id[8];             // System identifier string (always "FAT32 ")
};

struct PACKED fat32_record
{
    struct fat_bpb bpb;
    struct fat_ebr ebr;
};

struct PACKED fat_dirent
{
    uint8_t name[8]; // 8.3 format
    uint8_t ext[3];
    uint8_t attr;
    uint8_t user_attr;

    int8_t undel;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_u;

    uint16_t mod_time;
    uint16_t mod_date;
    uint16_t cluster;
    uint32_t file_sz;
};

struct fat32_volume
{
    struct fat32_record record;
    struct file* device;
};

// Number of characters one LFN can hold
#define LFN_LEN 13

struct PACKED fat_lfn
{
    uint8_t  order;
#define LFN_CHARS1 5
    uint16_t chars1[LFN_CHARS1];
    uint8_t  attr;
    uint8_t  type;
    uint8_t  checksum;
#define LFN_CHARS2 6
    uint16_t chars2[LFN_CHARS2];
    uint16_t zero;
#define LFN_CHARS3 2
    uint16_t chars3[LFN_CHARS3];
};

#define FAT_ATTR_RO             0x01
#define FAT_ATTR_HIDDEN         0x02
#define FAT_ATTR_SYS            0x04
#define FAT_ATTR_VOLID          0x08
#define FAT_ATTR_DIR            0x10
#define FAT_ATTR_AR             0x20
#define FAT_ATTR_LFN            0x0f

struct dirent;
struct inode;
struct file;
struct dentry;

void fat_init();

ssize_t fat_read(struct file* file, void* buf, off_t off, size_t size);
ssize_t fat_write(struct file* file, const void* buf, off_t off, size_t size);
int fat_lookup(struct inode* dir, const char* name, struct dentry* dentry);
ssize_t fat_getdents(struct inode* dir, off_t off, size_t size, struct dirent* dirp);
int fat_mkfile(struct inode* dir, const char* name, mode_t mode, uid_t uid, gid_t gid);
int fat_mkdir(struct inode* dir, const char* name, mode_t mode, uid_t uid, gid_t gid);
int fat_rm(struct inode* dir, const char* name);
int fat_unlink(struct inode* dir, const char* name);
