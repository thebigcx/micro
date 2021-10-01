#include <micro/devfs.h>
#include <micro/vfs.h>
#include <micro/list.h>
#include <micro/stdlib.h>

static struct list  devices;

struct file* devfs_find(struct file* dir, const char* name)
{
    LIST_FOREACH(&devices)
    {
        struct dentry* dev = node->data;
        if (!strcmp(name, dev->name))
        {
            struct file* copy = kmalloc(sizeof(struct file));
            memcpy(copy, dev->file, sizeof(struct file));
            return copy;
        }
    }

    return NULL;
}

ssize_t devfs_getdents(struct file* dir, off_t off, size_t size, struct dirent* dirp)
{
    size_t i;
    for (i = 0; i < size; i++)
    {
        if (i + off == devices.size) break;

        struct dentry* dev = list_get(&devices, i + off);
        strcpy(dirp[i].d_name, dev->name);
    }

    return i;
}

int devfs_mount(const char* dev, const void* data, struct file* fsroot)
{
    (void)dev; (void)data;

    memset(fsroot, 0, sizeof(struct file));

    fsroot->perms        = 0755;
    fsroot->type         = S_IFDIR;
    fsroot->ops.find     = devfs_find;
    fsroot->ops.getdents = devfs_getdents;

    return 0;
}

void devfs_init()
{
    devices = list_create();

    vfs_register_fs("devfs", devfs_mount);
    vfs_mount_fs("", "/dev", "devfs", NULL);
}

void devfs_register(struct file* file, const char* name)
{
    struct dentry* dentry = kmalloc(sizeof(struct dentry));

    strcpy(dentry->name, name);
    dentry->file = file;

    list_enqueue(&devices, dentry);
}