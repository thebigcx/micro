#pragma once

// TODO: move into kernel module

#include <micro/types.h>

// BIOS parameter block
struct __attribute__((packed)) fat_bpb
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
struct __attribute__((packed)) fat_ebr
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

struct __attribute__((packed)) fat32_record
{
    struct fat_bpb bpb;
    struct fat_ebr ebr;
};

struct __attribute__((packed)) fat_dirent
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

struct dirent;

void fat_init();

ssize_t fat_read(struct file* file, void* buf, off_t off, size_t size);
struct file* fat_find(struct file* dir, const char* name);
int fat_readdir(struct file* dir, size_t size, struct dirent* dirent);
void fat_mkfile(struct file* dir, const char* name);
void fat_mkdir(struct file* dir, const char* name);
void fat_rm(struct file* dir, const char* name);