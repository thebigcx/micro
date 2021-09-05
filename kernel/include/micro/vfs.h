#pragma once

#include <micro/types.h>

struct file;

typedef ssize_t (*read_t)(struct file* file, void* buf, off_t off, size_t size);
typedef ssize_t (*write_t)(struct file* file, const void* buf, off_t off, size_t size);
typedef struct file* (*find_t)(struct file* dir, const char* name);

struct file_ops
{
    read_t read;
    write_t write;
    find_t find;
};

#define FL_FILE     1
#define FL_DIR      2
#define FL_CHARDEV  3
#define FL_BLOCKDEV 4
#define FL_FIFO     5
#define FL_SOCKET   6
#define FL_SYMLINK  7
#define FL_MNTPT    8

// TODO: flags, modes, etc
struct fd
{
    struct file* filp;
    off_t off;
};

struct file
{
    char name[64];
    uint64_t inode;
    void* device;
    uint32_t flags;
    size_t size;

    struct file_ops ops;
};

void vfs_init();

ssize_t vfs_read(struct file* file, void* buf, off_t off, size_t size);
ssize_t vfs_write(struct file* file, void* buf, off_t off, size_t size);
struct file* vfs_find(struct file* dir, const char* name);

int vfs_addnode(struct file* file, const char* path);
struct file* vfs_getmnt(const char* path, char** relat);

void vfs_mount_fs(const char* dev, const char* mnt, const char* fs, void* data);

struct fd* vfs_open(struct file* file);
void vfs_close(struct fd* fd);

char* vfs_mkcanon(const char* path, const char* work);

struct file* vfs_resolve(const char* path);

int vfs_access(const char* path, int mode);

typedef struct file* (*mount_t)(const char*, void* data);

struct fs_type
{
    char* name; // e.g. 'ext2', 'fat32', 'initramfs'
    mount_t mount; // returns root file
};

void vfs_register_fs(char* fs, mount_t mount);