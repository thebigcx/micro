#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

char* uuidtostr(uint8_t uuid[16], char* buf)
{
    // Reorder
    uint8_t tmp[16];
    memcpy(tmp, uuid, 10);

    // From big-endian to little-endian
    tmp[8] = uuid[9];
    tmp[9] = uuid[8];

    tmp[10] = uuid[13];
    tmp[11] = uuid[12];
    tmp[12] = uuid[11];
    tmp[13] = uuid[10];
    tmp[14] = uuid[15];
    tmp[15] = uuid[14];
    
    // Convert to string
    snprintf(buf, 37, "%08x-%04x-%04x-%04x-%08x%04x",
            *(uint32_t*)&tmp[0], *(uint16_t*)&tmp[4], *(uint16_t*)&tmp[6],
            *(uint16_t*)&tmp[8], *(uint32_t*)&tmp[10], *(uint16_t*)&tmp[14]);
    return buf;
}

struct gpt_header
{
    char sig[8];
    uint32_t rev;
    uint32_t size;
    uint32_t hdr_checksum;
    uint32_t res;
    uint64_t hdrlba;
    uint64_t mirrorlba;
    uint64_t firstblk;
    uint64_t lastblk;
    uint8_t  guid[16];
    uint64_t partarray;
    uint32_t partcnt;
    uint32_t partsize;
    uint32_t array_checksum;

    /* blocksize - 0x5c zeroed */
};

struct gpt_entry
{
    uint8_t  type[16];
    uint8_t  partguid[16];
    uint64_t start;
    uint64_t end;
    uint64_t attr;
    uint16_t name[36];
};

static int device;
static const char* devpath;

void readblk(void* buf, uint64_t lba)
{
    lseek(device, lba * 512, SEEK_SET);
    read(device, buf, 512);
}

// Human-readable sizes
const char* sizes[] =
{
    "B", "KB", "MB", "GB", "TB", "PB"
};

// Print a size in a human-readable manor
// E.g. 160 KB, 16 GB
char* print_size(char* buf, size_t n, size_t size)
{
    // Get nearest multiple of 1000
    size_t i;
    for (i = 0; size / 1000; i++)
        size /= 1000;

    snprintf(buf, n, "%ld %s", size, sizes[i]);
    return buf;
}

static int unused_entry(struct gpt_entry* ent)
{
    for (size_t i = 0; i < 16; i++)
        if (ent->type[i]) return 0;
    return 1;
}

struct gpt_type
{
    const char* type;
    const char* uuid;
};

struct gpt_type types[] =
{
    { "Ext2 Filesystem", "0fc63daf-8483-4772-8e79-3d69d8477de4" },
    { "EFI System",      "c12a7328-f81f-11d2-ba4b-00a0c93ec93b" },
};

const char* type_uuid(uint8_t uuid[16])
{
    char buf[37];

    for (size_t i = 0; i < sizeof(types) / sizeof(struct gpt_type); i++)
    {
        if (!strcmp(uuidtostr(uuid, buf), types[i].uuid))
            return types[i].type;
    }
    return NULL;
}

void print_gpt()
{
    char str[32];

    void* buf = malloc(512);
   
    readblk(buf, 1);

    struct gpt_header hdr;
    memcpy(&hdr, buf, sizeof(struct gpt_header));

    printf("\033[93m%s\n\033[0m", devpath);    
    printf("GPT formatted disk\n");
    printf("Size: %s\n", print_size(str, 32, hdr.lastblk * 512));

    char* name = malloc(strlen(devpath) + 2);
    strcpy(name, devpath);
    strcat(name, "0");

    size_t blk = 2, off = 0;
    readblk(buf, blk);

    printf("\033[92m%-10s %-7s %s\n\033[0m", "Device", "Size", "Type");

    for (size_t i = 0; i < hdr.partcnt; i++)
    {
        if (off >= 512)
        {
            readblk(buf, ++blk);
            off = 0;
        }
        
        struct gpt_entry ent;
        memcpy(&ent, buf + off, hdr.partsize);
        
        off += hdr.partsize;
        
        if (unused_entry(&ent)) continue;
        
        name[strlen(name) - 1] = i + 1 + '0';
        
        printf("%-10s ", name);
        printf("%-7s ", print_size(str, 32, (ent.end - ent.start) * 512));
        printf("%s\n", type_uuid(ent.type));

        //TODO: more than 9 partitions 
    }

    free(buf);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: fdisk <device>\n");
        return -1;
    }

    devpath = argv[1];
    
    if ((device = open(devpath, O_RDONLY)) < 0)
    {
        perror("fdisk");
        return -1;
    }
   
    printf("\n"); 
    print_gpt();
    printf("\n");

    close(device);

    return 0;
}
