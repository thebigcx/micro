#include <micro/part.h>
#include <micro/heap.h>
#include <micro/devfs.h>

static ssize_t part_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct partdev* dev = file->inode->priv;
    return dev->dev.ops.read(&dev->dev, buf, off + dev->start * 512, size);
}

static ssize_t part_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct partdev* dev = file->inode->priv;
    return dev->dev.ops.write(&dev->dev, buf, off + dev->start * 512, size);
}

static struct file_ops partops =
{
    .read = part_read,
    .write = part_write
};

// Register a partition
void register_part(uint64_t start, uint64_t end, struct file* dev, const char* name)
{
    struct partdev* part = kmalloc(sizeof(struct partdev));
    
    part->start = start;
    part->end   = end;
    part->dev   = *dev;
    
    devfs_register_chrdev(&partops, name, 0660, part);
}
