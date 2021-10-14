#include <micro/mbr.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/stdlib.h>
#include <micro/part.h>
#include <micro/debug.h>

void mbr_init(const char* dev)
{
    struct file file;
    vfs_open(dev, &file, O_RDONLY);

    void* buf = kmalloc(512);
    file.ops.read(&file, buf, 0, 512);

    struct mbr mbr;
    memcpy(&mbr, buf + 0x1b8, sizeof(struct mbr));
    
    char* name = kmalloc(strlen(dev) + 2);
    strcpy(name, strrchr(dev, '/') + 1);
    strcat(name, "0");

    for (size_t i = 0; i < 4; i++)
    {
        if (!mbr.parts[i].type) continue;

        printk("MBR: found partition: %d-%d\n", mbr.parts[i].start, mbr.parts[i].start + mbr.parts[i].size);
        
        name[strlen(name) - 1] = i + 1 + '0';
        
        register_part(mbr.parts[i].start, mbr.parts[i].start + mbr.parts[i].size, &file, name);
    }
}
