#pragma once

#include <types.h>

struct file;

typedef ssize_t (*read_t)(struct file* file, void* buf, off_t off, size_t size);
typedef ssize_t (*write_t)(struct file* file, const void* buf, off_t off, size_t size);

struct file_ops
{
    read_t read;
    write_t write;
};

#define FL_FILE     1
#define FL_DIR      2
#define FL_CHARDEV  3
#define FL_BLOCKDEV 4
#define FL_FIFO     5
#define FL_SOCKET   6
#define FL_SYMLINK  7

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

int vfs_mount(struct file* file, const char* path);
struct file* vfs_getmnt(const char* path, char** relat);

struct fd* vfs_open(struct file* file);
void vfs_close(struct fd* fd);

struct file* vfs_resolve(const char* path);