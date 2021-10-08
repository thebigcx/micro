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
    size = min(size, file->inode->size - off);
    
    memcpy(buf, (void*)((uintptr_t)fb.addr + off), size);

    return size;
}

ssize_t fb_write(struct file* file, const void* buf, off_t off, size_t size)
{
    printk("fb write\n");
    size = min(size, file->inode->size - off);
    
    memcpy((void*)((uintptr_t)fb.addr + off), buf, size);

    return size;
}

int fb_ioctl(struct file* file, int req, void* argp)
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

int fb_mmap(struct file* file, struct vm_area* area)
{
    for (uintptr_t i = 0; i < (area->end - area->base) / PAGE4K; i++)
    {
        mmu_map(task_curr()->vm_map, area->base + i * PAGE4K, fb.phys + i * PAGE4K, PAGE_PR | PAGE_RW | PAGE_USR);
    }

    return 0;
}

void fb_init_dev()
{
    struct file_ops ops =
    {
        .read = fb_read,
        .write = fb_write,
        .ioctl = fb_ioctl,
        .mmap = fb_mmap,
    };

    devfs_register_chrdev(&ops, "fb0", 0660, NULL);
}