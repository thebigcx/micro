#include <init.h>
#include <debug/syslog.h>

static uintptr_t initrd_start;
static uintptr_t initrd_end;

struct fheader
{
    char name[128];
    uint64_t size;
};

void* initrd_read(const char* file)
{
    struct fheader* curr = (struct fheader*)initrd_start;

    while ((uintptr_t)curr < initrd_end)
    {
        if (!strcmp(curr->name, file))
        {
            void* buffer = kmalloc(curr->size);
            memcpy(buffer, (uintptr_t)curr + sizeof(struct fheader), curr->size);
            return buffer;
        }

        curr = (struct fheader*)((uintptr_t)curr + sizeof(struct fheader) + curr->size);
    }

    return NULL;
}

void initrd_init(uintptr_t start, uintptr_t end)
{
    initrd_start = start;
    initrd_end = end;
}
