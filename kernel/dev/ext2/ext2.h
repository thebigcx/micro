#pragma once

#include <micro/types.h>
#include <micro/vfs.h>
#include <micro/platform.h>

struct PACKED ext2_sb
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

struct PACKED ext2_sbext
{
    uint32_t first_inode;           // First non-reserved inode
    uint16_t inode_sz;              // Size of inode structure in bytes
    uint16_t block_group;           // Block group that this superblock is part of
    uint32_t features_compat;       // Optional features present
    uint32_t features_req;          // Required features present
    uint32_t features_ro;           // Read-only features present
    uint8_t  fs_id[16];             // Filesystem ID
    char     volume_name[16];       // Volume name
    char     last_mnt[64];          // Path this volume was last mounted to
    uint32_t alg_bitmap;            // Compression algorithms used
    uint8_t  prealloc_blocks;       // Number of blocks to preallocate for files
    uint8_t  prealloc_dir_blocks;   // Number of blocks to preallocate for directories
    uint16_t unused;                // Unused
    uint8_t  padding[16];           // Padding
    char     journal_id[16];        // Journal ID
    uint32_t journal_inode;         // Journal inode
    uint32_t journal_dev;           // Journal device
    uint32_t orpan_inode_head;      // Head of orphan inode list
};

struct PACKED ext2_bgd
{
    uint32_t block_bmp;     // Block address of block usage bitmap
    uint32_t inode_bmp;     // Block address of inode usage bitmap
    uint32_t inode_tbl;     // Starting block address of inode table
    uint32_t free_blocks;   // Number of free blocks in group
    uint32_t free_inodes;   // Number of free inodes in group
    uint32_t dir_cnt;       // Number of directories in group
    uint16_t padding;
    uint8_t res[12];
};

struct PACKED ext2_inode
{
    uint16_t mode;          // Types and Permissions
    uint16_t userid;        // User ID
    uint32_t size;          // Lower 32 bits of size
    uint32_t last_access;   // Last access time in POSIX time
    uint32_t creation_time; // Creation time in POSIX time
    uint32_t last_mod_time; // Last modification time in POSIX time
    uint32_t del_time;      // Deletion time in POSIX time
    uint16_t grpid;         // Group ID
    uint16_t link_cnt;      // Amount of hard links (directory entries)
    uint32_t sector_cnt;    // Count of disk sectors
    uint32_t flags;         // Flags
    uint32_t os_spec1;      // OS-specific value #1
    uint32_t directs[12];   // Direct block pointers

    uint32_t sind;          // Singly-indirect block pointers
    uint32_t dind;          // Doubly-indirect block pointers
    uint32_t tind;          // Triply-indirect block pointers

    uint32_t gen_num;       // Generation number
    uint32_t file_acl;      // Extended attributes for file
    
    union
    {
        uint32_t dir_acl;   // Directory attributes
        uint32_t size_u;    // File size upper 32 bits
    };

    uint32_t frag_addr;     // Block address of fragment
    uint8_t os_spec2[12];   // OS-specific value #2
};

#define INODE_FIFO      0x1000
#define INODE_CHARDEV   0x2000
#define INODE_DIR       0x4000
#define INODE_BLOCKDEV  0x6000
#define INODE_FILE      0x8000
#define INODE_SYMLINK   0xa000
#define INODE_SOCKET    0xc000

struct PACKED ext2_dirent
{
    uint32_t inode;      // Inode
    uint16_t size;       // Total size of this field
    uint8_t  name_len;   // Name length least-significant 8 bits
    uint8_t  type;       // Type indicator
    char     name[];
};

#define DIRENT_UNK      0
#define DIRENT_FILE     1
#define DIRENT_DIR      2
#define DIRENT_CHARDEV  3
#define DIRENT_BLOCKDEV 4
#define DIRENT_FIFO     5
#define DIRENT_SOCKET   6
#define DIRENT_SYMLINK  7

struct PACKED ext2_sbfull
{
    struct ext2_sb    sb;
    struct ext2_sbext sbext;
};

struct ext2_volume
{
    struct file*     device;
    size_t           blksize;

    struct ext2_bgd* groups;
    unsigned int     group_cnt;

    struct PACKED
    {
        struct ext2_sb    sb;
        struct ext2_sbext sbext;
    };
};

struct file;

void ext2_init();

ssize_t ext2_read(struct file* file, void* buf, off_t off, size_t size);
ssize_t ext2_write(struct file* file, const void* buf, off_t off, size_t size);
struct file* ext2_find(struct file* dir, const char* name);
int ext2_readdir(struct file* dir, size_t size, struct dirent* dirent);
ssize_t ext2_getdents(struct file* dir, off_t off, size_t n, struct dirent* dirp);
void ext2_mkfile(struct file* dir, const char* name);
void ext2_mkdir(struct file* dir, const char* name);
void ext2_rm(struct file* dir, const char* name);
void ext2_mknod(struct file* dir, struct file* file);