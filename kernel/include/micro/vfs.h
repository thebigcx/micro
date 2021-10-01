#pragma once

#include <micro/types.h>
#include <micro/fs.h>

struct file;
struct vm_area;

// TODO: should all return int's (errors), and store result in pointer
// TODO: function called sync(), which will sync the file to the inode on disk, reducing extra unneeded functions like chmod(), and symlink()
typedef struct fd*   (*open_t    )(struct file* file, uint32_t flags, mode_t mode);
typedef void         (*close_t   )(struct fd* fd);
typedef ssize_t      (*read_t    )(struct file* file, void* buf, off_t off, size_t size);
typedef ssize_t      (*write_t   )(struct file* file, const void* buf, off_t off, size_t size);
typedef int          (*ioctl_t   )(struct file* file, unsigned long req, void* argp);
typedef struct file* (*find_t    )(struct file* dir, const char* name);
typedef ssize_t      (*getdents_t)(struct file* dir, off_t off, size_t n, struct dirent* dirp);
typedef void         (*mkfile_t  )(struct file* dir, const char* name, mode_t mode, uid_t uid, gid_t gid);
typedef void         (*mkdir_t   )(struct file* dir, const char* name, mode_t mode, uid_t uid, gid_t gid);
typedef void         (*mknod_t   )(struct file* dir, const char* name, mode_t mode, dev_t dev, uid_t uid, gid_t gid);
typedef void         (*unlink_t  )(struct file* dir, const char* name);
typedef void         (*mmap_t    )(struct file* file, struct vm_area* area);
typedef int          (*chmod_t   )(struct file* file, mode_t mode);
typedef int          (*chown_t   )(struct file* file, uid_t uid, gid_t gid);
typedef int          (*readlink_t)(struct file* file, char* buf, size_t n);
typedef int          (*symlink_t )(struct file* file, const char* link);
typedef int          (*link_t    )(struct file* old, const char* new, struct file* dir);

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
    chmod_t    chmod;
    chown_t    chown;
    readlink_t readlink;
    symlink_t  symlink;
    link_t     link;
};

#define S_IFMT   (0xf000)

#define S_IFIFO  (0x1000)
#define S_IFCHR	 (0x2000)
#define S_IFDIR	 (0x4000)
#define S_IFBLK	 (0x6000)
#define S_IFREG	 (0x8000)
#define S_IFLNK	 (0xa000)
#define S_IFSOCK (0xc000)

#define S_IRWXU  (00700)
#define S_IRUSR  (00400)
#define S_IWUSR  (00200)
#define S_IXUSR  (00100)

#define S_IRWXG  (00070)
#define S_IRGRP  (00040)
#define S_IWGRP  (00020)
#define S_IXGRP  (00010)

#define S_IRWXO  (00007)
#define S_IROTH  (00004)
#define S_IWOTH  (00002)
#define S_IXOTH  (00001)

#define S_ISUID  (04000)
#define S_ISGID  (02000)
#define S_ISVTX  (01000)

#define S_PERMS  (0x0fff)

#define S_ISFIFO(m) ((m & S_IFMT) == S_IFIFO)
#define S_ISCHR(m)  ((m & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)  ((m & S_IFMT) == S_IFDIR)
#define S_ISBLK(m)  ((m & S_IFMT) == S_IFBLK)
#define S_ISREG(m)  ((m & S_IFMT) == S_IFREG)
#define S_ISLNK(m)  ((m & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) ((m & S_IFMT) == S_IFSOCK)

#define CHECK_RPERM(file) if (vfs_checkperm(file, 04) == -1) return -EACCES;
#define CHECK_WPERM(file) if (vfs_checkperm(file, 02) == -1) return -EACCES;
#define CHECK_XPERM(file) if (vfs_checkperm(file, 01) == -1) return -EACCES;

// TODO: flags, modes, etc
struct fd // TODO: rename to 'file'
{
    struct file* filp;
    off_t        off;
    uint32_t     flags;
};

struct dentry
{
    char name[256];
    struct file* file;
};

struct file // TODO: rename to 'inode'
{
    uint64_t        inode;
    void*           device;
    //uint32_t        type;
    //uint32_t        perms;
    mode_t          mode;
    size_t          size;
    struct file_ops ops;    // TODO: remove
    struct file*    parent; // TODO: remove
    dev_t           major;
    dev_t           minor;
    unsigned int    links;
    time_t          atime;
	time_t          mtime;
	time_t          ctime;

    uid_t           uid;
    gid_t           gid;
};

struct mount
{
    const char* path;
    struct file* file;
};

struct dirent;

struct file* vfs_create_file();

void vfs_init();

ssize_t vfs_read(struct file* file, void* buf, off_t off, size_t size);
ssize_t vfs_write(struct file* file, const void* buf, off_t off, size_t size);
struct file* vfs_find(struct file* dir, const char* name);
ssize_t vfs_getdents(struct file* dir, off_t off, size_t n, struct dirent* dirp);

int vfs_mkfile(const char* path, mode_t mode, uid_t uid, gid_t gid);
int vfs_mkdir(const char* name, mode_t mode, uid_t uid, gid_t gid);
int vfs_mknod(const char* path, mode_t mode, dev_t dev, uid_t uid, gid_t gid);

int vfs_unlink(const char* pathname);

int vfs_ioctl(struct file* file, unsigned long req, void* argp);

struct file* vfs_getmnt(const char* path, char** relat);

int vfs_mount_fs(const char* dev, const char* mnt,
                 const char* fs, const void* data);
int vfs_umount_fs(const char* mnt);

struct fd* vfs_open(struct file* file, uint32_t flags, mode_t mode);
void vfs_close(struct fd* fd);

char* vfs_mkcanon(const char* path, const char* work);

int vfs_resolve(const char* path, struct file* out, int symlinks);

int vfs_access(const char* path, int mode);

void vfs_mmap(struct file* file, struct vm_area* area);

int vfs_chmod(struct file* file, mode_t mode);
int vfs_chown(struct file* file, uid_t uid, gid_t gid);

int vfs_checkperm(struct file* file, unsigned int mask);

int vfs_readlink(struct file* file, char* buf, size_t n);
int vfs_symlink(const char* target, const char* link);
int vfs_link(const char* old, const char* new);
int vfs_rename(const char* old, const char* new);

int vfs_rmdir(const char* path);

typedef int (*mount_t)(const char*, const void* data, struct file* fsroot);

struct fs_type
{
    char* name; // e.g. 'ext2', 'fat32', 'initramfs'
    mount_t mount; // returns root file
};

void vfs_register_fs(char* fs, mount_t mount);