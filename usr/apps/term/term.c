#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <pty.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

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
static void*           fbaddr;
static void*           fbend;

static unsigned int cx = 0;
static unsigned int cy = 0;

static pid_t sh_pid;

static int ptm, pts;

static void load_font()
{
    int fnt = open("/usr/share/font.psf", O_RDONLY, 0);

    read(fnt, &font.hdr, sizeof(struct psf_header));

    font.buffer = malloc(font.hdr.ch_size * 256);
    read(fnt, font.buffer, font.hdr.ch_size * 256);

    close(fnt);
}

void clear(uint32_t bg)
{
    uint32_t* p = fbaddr;
    do *p = bg;
    while ((uintptr_t)++p < (uintptr_t)fbend);
}

void scroll_down()
{
    // TODO
    clear(0);
    cy = 0;
}

void newline()
{
    drawch(' ', 0, 0);
    cy++;
    cx = 0;

    if (cy == info.yres / 16)
    {
        scroll_down();
    }
}

void tab()
{
    for (size_t i = 0; i < 4 - (cx % 4); i++)
    {
        drawch(' ', 0, 0);
    }

    cx += 4 - (cx % 4);
    if (cx >= info.xres / 8)
    {
        newline();
    }

    draw_cursor();
}

void backspace()
{
    drawch(' ', 0x0, 0x0);
    cx--;
    drawch(' ', 0x0, 0x0);

    draw_cursor();
}

void draw_cursor()
{
    for (uint64_t j = 0; j < 16; j++)
    for (uint64_t i = 0; i < 8; i++)
    {
        uint32_t x = (cx * 8 ) + i;
        uint32_t y = (cy * 16) + j;
        *((uint32_t*)fbaddr + x + y * info.xres) = 0xffffffff;
    }
}

void putch(char c, uint32_t fg, uint32_t bg)
{
    if (c == '\n')
    {
        newline();
        draw_cursor();
        return;
    }
    if (c == '\t')
    {
        tab();
        return;
    }
    if (c == '\b')
    {
        backspace();
        return;
    }

    if (c < 32)
    {
        /*drawch(' ', 0, 0);
        cx++;
        if (cx == info.xres / 8)
        {
            cx = 0;
            newline();
        }
        draw_cursor();*/
        return;
    }

    drawch(c, fg, bg);

    cx++;
    if (cx == info.xres / 8) newline();

    draw_cursor();
}

void drawch(char c, uint32_t fg, uint32_t bg)
{
    char* face = (char*)font.buffer + (c * font.hdr.ch_size);

    uint32_t depth = info.bpp / 8;

    for (uint64_t j = 0; j < 16; j++)
    {
        for (uint64_t i = 0; i < 8; i++)
        {
            uint32_t x = (cx * 8 ) + i;
            uint32_t y = (cy * 16) + j;

            uint32_t col = (*face & (0b10000000 >> i)) > 0
                         ? fg : bg;

            *((uint32_t*)fbaddr + x + y * info.xres) = col;
        }
        face++;
    }
}

static char ascii[] =
{
    'c', '~', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't',
    'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n', '^', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\'', '`',
    '~', '\\', 'z', 'x', 'c', 'v', 'b',
    'n', 'm', ',', '.', '/', '~', '*',
    '~', ' ', '~', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c'
};

static char shift_ascii[] =
{
    'c', '~', '!', '@', '#', '$', '%',
    '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'U', 'O', 'P', '{', '}',
    '\n', '^', 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '"', '~',
    '~', '|', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', '<', '>', '?', '~', '*',
    '~', ' ', '~', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c',
    'c', 'c', 'c', 'c', 'c', 'c', 'c'
};

static int ctrl = 0;
static int shift = 0;

void handle_kb(int sc)
{
    if (sc == 29)
    {
        ctrl = 1;
        return;
    }
    else if (sc == 0x9d)
    {
        ctrl = 0;
        return;
    }

    if (sc == 0x2a)
    {
        shift = 1;
        return;
    }
    else if (sc == 0xaa)
    {
        shift = 0;
        return;
    }

    if (sc < 88)
    {
        char ch = shift ? shift_ascii[sc] : ascii[sc];

        if (ctrl)
        {
            char c = '^';
            write(ptm, &c, 1);
            write(ptm, &ch, 1);
            return;
        }

        write(ptm, &ch, 1);
    }

}

int main(int argc, char** argv)
{
    load_font();

    //putenv("HOME=/root");

    shift = 0;

    fb = open("/dev/fb0", O_RDWR, 0);

    ioctl(fb, FBIOGINFO, &info);

    size_t size = info.xres * info.yres * (info.bpp / 8);

    fbaddr = mmap(0x1000000, size, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE, fb, 0);
    fbend = (void*)((uintptr_t)fbaddr + size);

    char pts_name[128];
    openpty(&ptm, &pts, pts_name, NULL, NULL);

    struct winsize winsize =
    {
        .ws_col = info.xres / 8,
        .ws_row = info.yres / 16,
        .ws_xpixel = info.xres,
        .ws_ypixel = info.yres
    };

    ioctl(pts, TIOCSWINSZ, &winsize);

    dup2(pts, 0);
    dup2(pts, 1);
    dup2(pts, 2);

    setvbuf(stdout, NULL, _IONBF, 0);

    sh_pid = fork();
    if (sh_pid == 0)
    {
        const char* argv[] = { "/usr/bin/bash", NULL };
        const char* envp[] = { NULL };

        execve(argv[0], argv, envp);
        for (;;);
    }

    int kb = open("/dev/keyboard", O_RDONLY, 0);

    while (1)
    {
        //printf("psadas");
        char c;
        if (read(ptm, &c, 1))
        {
            putch(c, 0xffffffff, 0);
        }

        int sc;
        if (read(kb, &sc, 1))
        {
            handle_kb(sc);
        }

        int status;
        if (waitpid(sh_pid, &status, WNOHANG) > 0)
            break;
    }

    return 0;
}
