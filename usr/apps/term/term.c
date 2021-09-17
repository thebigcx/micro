#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <micro/fb.h>

struct psf_header
{
    uint8_t magic[2];
    uint8_t mode;
    uint8_t ch_size; // Character size
};

struct psf_font
{
    struct psf_header hdr;
    void*             buffer;
};

static struct psf_font font;
static struct fbinfo   info;
static int             fb;

static unsigned int cx = 0;
static unsigned int cy = 0;

static void load_font()
{
    int fnt = open("/usr/share/font.psf", O_RDONLY, 0);

    read(fnt, &font.hdr, sizeof(struct psf_header));

    font.buffer = malloc(font.hdr.ch_size * 256);
    read(fnt, font.buffer, font.hdr.ch_size * 256);

    close(fnt);
}

void putch(char c, uint32_t fg, uint32_t bg)
{
    char* face = (char*)font.buffer + (c * font.hdr.ch_size);

    uint32_t depth = info.bpp / 8;

    for (uint64_t j = 0; j < 16; j++)
    {
        for (uint64_t i = 0; i < 8; i++)
        {
            if ((*face & (0b10000000 >> i)) > 0)
            {
                uint32_t x = (cx * 8) + i;
                uint32_t y = (cy * 8) + j;
                pwrite(fb, &fg, 4, (x * depth) + (y * depth * info.xres));
            }
        }
        face++;
    }

    cx++;
}

int main(int argc, char** argv)
{
    load_font();

    fb = open("/dev/fb0", O_RDWR, 0);

    ioctl(fb, FBIOGINFO, &info);

    putch('c', 0xffffffff, 0);
    putch('c', 0xffffffff, 0);
    putch('c', 0xffffffff, 0);
    putch('c', 0xffffffff, 0);
    putch('c', 0xffffffff, 0);
    putch('c', 0xffffffff, 0);
    putch('c', 0xffffffff, 0);



    for (;;);
    return 0;
}