#include <micro/gpt.h>
#include <micro/try.h>
#include <micro/heap.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/stdlib.h>
#include <micro/devfs.h>
#include <micro/debug.h>

struct gpt_partdev
{
    uint64_t start, end; // Start and end LBAs   
    struct file dev;
};

static ssize_t part_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct gpt_partdev* dev = file->inode->priv;
    return dev->dev.ops.read(&dev->dev, buf, off + dev->start * 512, size);
}

static ssize_t part_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct gpt_partdev* dev = file->inode->priv;
    return dev->dev.ops.write(&dev->dev, buf, off + dev->start * 512, size);
}

static struct file_ops partops =
{
    .read = part_read,
    .write = part_write
};

static struct gpt_partdev* mkpartdev(uint64_t start, uint64_t end, struct file* dev)
{
    struct gpt_partdev* part = kcalloc(sizeof(struct gpt_partdev));
    part->start = start;
    part->end   = end;
    part->dev   = *dev;
    return part;
}

static int unused(struct gpt_entry* ent)
{
    for (size_t i = 0; i < 16; i++)
        if (ent->type[i]) return 0;
    return 1;
}

int gpt_init(const char* dev)
{
    struct file file;
    TRY(vfs_open(dev, &file, O_RDONLY));
    
    // TODO: block device should have a 'blocksize' field
    void* buf = kmalloc(512);

    file.ops.read(&file, buf, 512, 512);
   
    struct gpt_header header;
    memcpy(&header, buf, sizeof(struct gpt_header));
    printk("partition count: %d\n", header.partcnt);

    // TODO: handle more than /dev/sda{1-9}

    char* name = kmalloc(strlen(dev) + 2);
    strcpy(name, strrchr(dev, '/') + 1);
    strcat(name, "0");

    // Loop through the partition table entries
    size_t blk = 0, off = 0;
    file.ops.read(&file, buf, 1024 + blk * 512, 512);

    for (size_t i = 0; i < header.partcnt; i++)
    {
        if (off >= 512)
        {
            file.ops.read(&file, buf, 1024 + ++blk * 512, 512);
            off = 0;
        }

        struct gpt_entry ent;
        memcpy(&ent, buf + off, header.partsize);
     
        off += header.partsize;

        if (unused(&ent)) continue; // Unused entry

        printk("found partition: ");
        for (size_t j = 0; j < 36; j++)
            printk("%c", ent.name[j]);
        printk(" %d-%d\n", ent.start, ent.end);

        struct gpt_partdev* part = mkpartdev(ent.start, ent.end, &file);
        
        name[strlen(name) - 1] = i + 1 + '0';
        printk("%s\n", name);
        devfs_register_blkdev(&partops, name, 0660, part);

    }

    kfree(buf);
    return 0;
}
