#pragma once

#include <micro/types.h>

struct __attribute__((packed)) ext2_sb_base
{
    uint32_t inode_cnt;         // Total number of inodes in filesystem
    uint32_t blk_cnt;           // Total number of blocks in filesystem
    uint32_t res_blocks;        // Number of blocks reserved for superuser
    uint32_t free_blocks;       // Number of unallocated blocks
    uint32_t free_inodes;       // Number of free inodes
    uint32_t first_dat_block;   // Block containing this superblock
    uint32_t log_block_sz;      // log2(block_sz) - 10
    uint32_t log_frag_sz;       // log2(frag_sz) - 10
    uint32_t blks_per_grp;      // Number of blocks per block group
    uint32_t frags_per_grp;     // Number of fragments per block group
    uint32_t inodes_per_grp;    // Number of inodes per block group
    uint32_t mnt_time;          // Last mount time (POSIX time)
    uint32_t write_time;        // Last write time (POSIX time)

    uint16_t mnt_cnt;           // Number of times volume has been mounted
    uint16_t max_mnt_cnt;       // Maximum mounts before consistency check
    uint16_t ext2_sig;          // Ext2 signature
    uint16_t state;             // Filesystem state
    uint16_t errors;            // What to do when error detected
    uint16_t min_version;       // Minor portion of version
    
    uint32_t last_check;        // POSIX time of last check
    uint32_t check_intvl;       // Interval (in POSIX time) between checks
    uint32_t os_id;             // Operating system ID of creation
    uint32_t maj_version;       // Major portion of version

    uint16_t res_uid;           // User ID that can use reserved blocks
    uint16_t res_gid;           // Group ID that can use reserved blocks
};

struct __attribute__((packed)) ext2_sb_ext
{
    uint32_t first_inode;           // First non-reserved inode
    uint16_t inode_sz;              // Size of inode structure in bytes
    uint16_t block_group;           // Block group that this superblock is part of
    uint32_t features_compat;       // Optional features present
    uint32_t features_req;          // Required features present
    uint32_t features_ro;           // Read-only features present
    uint8_t fs_id[16];              // Filesystem ID
    char volume_name[16];           // Volume name
    char last_mnt[64];              // Path this volume was last mounted to
    uint32_t alg_bitmap;            // Compression algorithms used
    uint8_t prealloc_blocks;        // Number of blocks to preallocate for files
    uint8_t prealloc_dir_blocks;    // Number of blocks to preallocate for directories
    uint16_t unused;                // Unused
    uint8_t padding[16];            // Padding
    char journal_id[16];            // Journal ID
    uint32_t journal_inode;         // Journal inode
    uint32_t journal_dev;           // Journal device
    uint32_t orpan_inode_head;      // Head of orphan inode list
};

struct __attribute__((packed)) ext2_sb
{
    struct ext2_sb_base sb;
    struct ext2_sb_ext sbext;
};

struct ext2_volume
{
    struct file* device;
};

struct file;

void ext2_init();