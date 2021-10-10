#pragma once

#include <micro/types.h>
#include <micro/fs.h>

struct inode;
struct vm_area;
struct dentry;
struct file;

struct file_ops
{
    int     (*open)(struct inode*, struct file*);
    int     (*close)(struct file*);
    ssize_t (*read)(struct file*, void*, off_t, size_t);
    ssize_t (*write)(struct file*, const void*, off_t, size_t);
    int     (*ioctl)(struct file*, int, void*);
    int     (*mmap)(struct file*, struct vm_area*);
    int     (*chmod)(struct file*, mode_t);
    int     (*chown)(struct file*, uid_t, gid_t);
};

struct inode_ops
{
    int (*lookup)(struct inode*, const char*, struct dentry*);
    int (*mknod)(struct inode*, const char*, mode_t, dev_t, uid_t, gid_t);
    int (*mkdir)(struct inode*, const char*, mode_t, uid_t, gid_t);

    ssize_t (*getdents)(struct inode*, off_t, size_t, struct dirent*);

    int (*unlink)(struct inode*, const char*);
    int (*readlink)(struct inode*, char*, size_t);
    int (*symlink)(struct inode*, const char*);
    int (*link)(struct inode*, const char*, struct inode*);

    int (*set_atime)(struct inode*, time_t);
    int (*set_mtime)(struct inode*, time_t);
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

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK     10
#define DT_SOCK    12

#define IFTODT(x) ((x) >> 12 & 017)

// TODO: flags, modes, etc
struct file // TODO: rename to 'file'
{
    struct inode*   inode;
    off_t           off;
    uint32_t        flags;
    struct file_ops ops;
};

struct dentry
{
    char name[256];
    struct inode* file;
};

struct inode
{
    mode_t          mode;
    size_t          size;
    dev_t           rdev;
    nlink_t         nlink;
    time_t          atime;
	time_t          mtime;
	time_t          ctime;
    uid_t           uid;
    gid_t           gid;
    blksize_t       blksize;
    blkcnt_t        blocks;

    ino_t           inode;
    void*           priv;

    struct inode_ops ops;
    struct file_ops  fops;
};

struct mount
{
    const char* path;
    struct inode* file;
};

struct dirent;

struct inode* vfs_create_file();

void vfs_init();

ssize_t vfs_read(struct file* file, void* buf, size_t size);
ssize_t vfs_write(struct file* file, const void* buf, size_t size);
ssize_t vfs_getdents(struct inode* dir, off_t off, size_t n, struct dirent* dirp);

ssize_t vfs_pread(struct file* file, void* buf, size_t size, off_t off);
ssize_t vfs_pwrite(struct file* file, const void* buf, size_t size, off_t off);

int vfs_mkdir(const char* name, mode_t mode, uid_t uid, gid_t gid);
int vfs_mknod(const char* path, mode_t mode, dev_t dev, uid_t uid, gid_t gid);

int vfs_unlink(const char* pathname);

int vfs_ioctl(struct file* file, int req, void* argp);

int vfs_getmnt(const char* path, char** relat, struct inode* out);

int vfs_mount_fs(const char* dev, const char* mnt,
                 const char* fs, const void* data);
int vfs_umount_fs(const char* mnt);

int vfs_open(const char* path, struct file* file, uint32_t flags);

void vfs_close(struct file* fd);

char* vfs_mkcanon(const char* path, const char* work);

int vfs_resolve(const char* path, struct inode* out, int symlinks);

int vfs_access(const char* path, int mode);

void vfs_mmap(struct file* file, struct vm_area* area);

int vfs_chmod(struct file* file, mode_t mode);
int vfs_chown(struct file* file, uid_t uid, gid_t gid);

int vfs_checkperm(struct inode* file, unsigned int mask);

int vfs_readlink(struct inode* inode, char* buf, size_t n);
int vfs_symlink(const char* target, const char* link);
int vfs_link(const char* old, const char* new);
int vfs_rename(const char* old, const char* new);

int vfs_rmdir(const char* path);

typedef int (*mount_t)(const char*, const void* data, struct inode* fsroot);

struct fs_type
{
    char* name; // e.g. 'ext2', 'fat32', 'initramfs'
    mount_t mount; // returns root file
};

void vfs_register_fs(char* fs, mount_t mount);
