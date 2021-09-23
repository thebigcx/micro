#pragma once

#include <micro/types.h>
#include <micro/fs.h>

struct file;
struct vm_area;

// TODO: should all return int's (errors), and store result in pointer
typedef struct fd*   (*open_t    )(struct file* file, uint32_t flags, mode_t mode);
typedef void         (*close_t   )(struct fd* fd);
typedef ssize_t      (*read_t    )(struct file* file, void* buf, off_t off, size_t size);
typedef ssize_t      (*write_t   )(struct file* file, const void* buf, off_t off, size_t size);
typedef int          (*ioctl_t   )(struct file* file, unsigned long req, void* argp);
typedef struct file* (*find_t    )(struct file* dir, const char* name);
typedef ssize_t      (*getdents_t)(struct file* dir, off_t off, size_t n, struct dirent* dirp);
typedef void         (*mkfile_t  )(struct file* dir, const char* name);
typedef void         (*mkdir_t   )(struct file* dir, const char* name);
typedef void         (*mknod_t   )(struct file* dir, struct file* file);
typedef void         (*unlink_t  )(struct file* dir, const char* name);
typedef void         (*mmap_t    )(struct file* file, struct vm_area* area);

struct file_ops
{
    open_t     open;
    close_t    close;
    read_t     read;
    write_t    write;
    ioctl_t    ioctl;
    find_t     find;
    getdents_t getdents;
    mkfile_t   mkfile;
    mkdir_t    mkdir;
    mknod_t    mknod;
    unlink_t   unlink;
    mmap_t     mmap;
};

#define FL_FIFO    (0x1000)
#define FL_CHRDEV  (0x2000)
#define FL_DIR     (0x4000)
#define FL_BLKDEV  (0x6000)
#define FL_FILE    (0x8000)
#define FL_SYMLINK (0xa000)
#define FL_SOCKET  (0xc000)

// TODO: flags, modes, etc
struct fd
{
    struct file* filp;
    off_t        off;
    uint32_t     flags;
};

struct file
{
    char            name[64];
    uint64_t        inode;
    void*           device;
    uint32_t        flags;
    size_t          size;
    struct file_ops ops;
    struct file*    parent;
    dev_t           major;
    dev_t           minor;
    unsigned int    links;
    time_t          atime;
	time_t          mtime;
	time_t          ctime;
};

struct dirent;

struct file* vfs_create_file();

void vfs_init();

ssize_t vfs_read(struct file* file, void* buf, off_t off, size_t size);
ssize_t vfs_write(struct file* file, const void* buf, off_t off, size_t size);
struct file* vfs_find(struct file* dir, const char* name);
ssize_t vfs_getdents(struct file* dir, off_t off, size_t n, struct dirent* dirp);

int vfs_mkfile(const char* path);
int vfs_mkdir(const char* name);

//void vfs_rm(struct file* dir, const char* name);
int vfs_unlink(const char* pathname);

int vfs_ioctl(struct file* file, unsigned long req, void* argp);

int vfs_addnode(struct file* file, const char* path);
void* vfs_rmnode(const char* path);
struct file* vfs_getmnt(const char* path, char** relat);

int vfs_mount_fs(const char* dev, const char* mnt,
                 const char* fs, const void* data);
int vfs_umount_fs(const char* mnt);

struct fd* vfs_open(struct file* file, uint32_t flags, mode_t mode);
void vfs_close(struct fd* fd);

char* vfs_mkcanon(const char* path, const char* work);

int vfs_resolve(const char* path, struct file* out);

int vfs_access(const char* path, int mode);

void vfs_mmap(struct file* file, struct vm_area* area);

typedef struct file* (*mount_t)(const char*, const void* data);

struct fs_type
{
    char* name; // e.g. 'ext2', 'fat32', 'initramfs'
    mount_t mount; // returns root file
};

void vfs_register_fs(char* fs, mount_t mount);