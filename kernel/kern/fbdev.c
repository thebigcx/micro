#include <micro/fbdev.h>
#include <micro/stdlib.h>
#include <micro/vfs.h>
#include <micro/fb.h>
#include <micro/errno.h>
#include <arch/mmu.h>
#include <micro/task.h>
#include <micro/devfs.h>

static struct fb fb;
static int ready = 0;

void fb_clear(uint32_t fg)
{
    memset((void*)fb.addr, fg, fb.width * fb.height * (fb.bpp / 8));
}

void fb_init(unsigned int width, unsigned int height, unsigned int depth)
{
    fb.width = width;
    fb.height = height;
    fb.bpp = depth;

    ready = 1;
}

void fb_set_addr(void* addr)
{
    fb.addr = addr;
}

void fb_set_phys(uintptr_t phys)
{
    fb.phys = phys;
}

ssize_t fb_read(struct file* file, void* buf, off_t off, size_t size)
{
    size = min(size, file->size - off);
    
    memcpy(buf, (void*)((uintptr_t)fb.addr + off), size);

    return size;
}

ssize_t fb_write(struct file* file, const void* buf, off_t off, size_t size)
{
    size = min(size, file->size - off);
    
    memcpy((void*)((uintptr_t)fb.addr + off), buf, size);

    return size;
}

int fb_ioctl(struct file* file, unsigned long req, void* argp)
{
    switch (req)
    {
        case FBIOGINFO:
        {
            struct fbinfo* inf = (struct fbinfo*)argp;
            inf->xres = fb.width;
            inf->yres = fb.height;
            inf->bpp  = fb.bpp;
            return 0;
        }
    }

    return -EINVAL;
}

void fb_mmap(struct file* file, struct vm_area* area)
{
    for (uintptr_t i = 0; i < (area->end - area->begin) / PAGE4K; i++)
    {
        mmu_map(task_curr()->vm_map, area->begin + i * PAGE4K, fb.phys + i * PAGE4K, PAGE_PR | PAGE_RW | PAGE_USR);
    }
}

void fb_init_dev()
{
    struct file* file = vfs_create_file();
    file->flags       = FL_CHARDEV;
    file->size        = fb.width * fb.height * (fb.bpp / 8);
    file->ops.read    = fb_read;
    file->ops.write   = fb_write;
    file->ops.ioctl   = fb_ioctl;
    file->ops.mmap    = fb_mmap;

    strcpy(file->name, "fb0");
    devfs_register(file);
}