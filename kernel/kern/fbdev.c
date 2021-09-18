#include <micro/fbdev.h>
#include <micro/stdlib.h>
#include <micro/vfs.h>
#include <micro/fb.h>
#include <micro/errno.h>

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

void fb_init_dev()
{
    struct file* file = vfs_create_file();
    file->flags       = FL_CHARDEV;
    file->size        = fb.width * fb.height * (fb.bpp / 8);
    file->ops.read    = fb_read;
    file->ops.write   = fb_write;
    file->ops.ioctl   = fb_ioctl;

    vfs_addnode(file, "/dev/fb0");
}