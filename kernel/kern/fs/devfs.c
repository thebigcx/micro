#include <micro/devfs.h>
#include <micro/vfs.h>
#include <micro/list.h>
#include <micro/stdlib.h>

static struct list  devices;
static struct file* devfs;

struct file* devfs_find(struct file* dir, const char* name)
{
    LIST_FOREACH(&devices)
    {
        struct file* dev = node->data;
        if (!strcmp(name, dev->name))
        {
            struct file* copy = kmalloc(sizeof(struct file));
            memcpy(copy, dev, sizeof(struct file));
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

        struct file* dev = list_get(&devices, i + off);
        strcpy(dirp[i].d_name, dev->name);
    }

    return i;
}

void devfs_init()
{
    devices = list_create();

    devfs = vfs_create_file();

    devfs->perms        = 0040;
    devfs->type         = FL_DIR;
    devfs->ops.find     = devfs_find;
    devfs->ops.getdents = devfs_getdents;

    vfs_addnode(devfs, "/dev");
}

void devfs_register(struct file* file)
{
    list_push_back(&devices, file);
}