#include <micro/devfs.h>
#include <micro/vfs.h>
#include <micro/list.h>
#include <micro/stdlib.h>
#include <micro/errno.h>
#include <micro/time.h>

static struct list  devices;

int devfs_lookup(struct inode* dir, const char* name, struct dentry* dentry)
{
    LIST_FOREACH(&devices)
    {
        struct dentry* dev = node->data;
        if (!strcmp(name, dev->name))
        {
            strcpy(dentry->name, name);
            dentry->file = memdup(dev->file, sizeof(struct inode));

            return 0;
        }
    }

    return -ENOENT;
}

ssize_t devfs_getdents(struct inode* dir, off_t off, size_t size, struct dirent* dirp)
{
    size_t i, bytes = 0;
    for (i = 0; i < size; i++)
    {
        if (i + off == devices.size) break;

        struct dentry* dev = list_get(&devices, i + off);
        
        strcpy(dirp[i].d_name, dev->name);
        dirp[i].d_type   = IFTODT(dev->file->mode & S_IFMT);
        dirp[i].d_reclen = sizeof(struct dirent);
        dirp[i].d_off    = sizeof(struct dirent) * (i + 1);

        bytes += sizeof(struct dirent);
    }

    return i;
}

int devfs_mount(const char* dev, const void* data, struct inode* fsroot)
{
    (void)dev; (void)data;

    memset(fsroot, 0, sizeof(struct inode));

    fsroot->mode         = S_IFDIR | 0755;
    fsroot->ops.lookup   = devfs_lookup;
    fsroot->ops.getdents = devfs_getdents;

    return 0;
}

void devfs_init()
{
    devices = list_create();

    vfs_register_fs("devfs", devfs_mount);
    vfs_mount_fs("", "/dev", "devfs", NULL);
}

void devfs_register(struct file_ops* ops, const char* name, mode_t mode, void* priv)
{
    struct inode* file = kcalloc(sizeof(struct inode));

    file->fops = *ops;
    file->mode = mode;
    file->priv = priv;
    file->atime = time_getepoch();
    file->mtime = file->atime;
    file->ctime = file->atime;

    struct dentry* dentry = kmalloc(sizeof(struct dentry));

    strcpy(dentry->name, name);
    dentry->file = file;

    list_enqueue(&devices, dentry);
}

void devfs_register_chrdev(struct file_ops* ops, const char* name, mode_t mode, void* priv)
{
    devfs_register(ops, name, mode | S_IFCHR, priv);
}

void devfs_register_blkdev(struct file_ops* ops, const char* name, mode_t mode, void* priv)
{
    devfs_register(ops, name, mode | S_IFBLK, priv);
}
