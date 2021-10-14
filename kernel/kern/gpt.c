#include <micro/gpt.h>
#include <micro/try.h>
#include <micro/heap.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/stdlib.h>
#include <micro/debug.h>
#include <micro/part.h>

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

        printk("GPT: found partition: ");
        for (size_t j = 0; j < 36; j++)
            printk("%c", ent.name[j]);
        printk(" %d-%d\n", ent.start, ent.end);

        name[strlen(name) - 1] = i + 1 + '0';
        register_part(ent.start, ent.end, &file, name);
    }

    kfree(buf);
    return 0;
}
