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
#include <ansi.h>

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

struct cell
{
    uint32_t fg, bg;
    char c;
    int dirty;
};

static struct cell* cells;
static int rows, cols;

static struct psf_font font;
static struct fbinfo   info;
static int             fb;
static void*           fbaddr;
static void*           fbend;

static unsigned int cx = 0;
static unsigned int cy = 0;

static pid_t sh_pid;

static int ptmfd, ptsfd;
static FILE* ptm, *pts;

static ansi_t ansi;

static void load_font()
{
    int fnt = open("/usr/share/font.psf", O_RDONLY, 0);

    read(fnt, &font.hdr, sizeof(struct psf_header));

    font.buffer = malloc(font.hdr.ch_size * 256);
    read(fnt, font.buffer, font.hdr.ch_size * 256);

    close(fnt);
}

struct cell* cellat(unsigned int x, unsigned int y)
{
    return &cells[y * cols + x];
}

void setcell(struct cell* cell, char c, uint32_t fg, uint32_t bg)
{
    cell->c     = c;
    cell->fg    = fg;
    cell->bg    = bg;
    cell->dirty = 1;
}

void draw_cell(unsigned int x, unsigned int y)
{
    struct cell* cell = cellat(x, y);

    char* face = (char*)font.buffer + (cell->c * font.hdr.ch_size);

    for (uint64_t j = 0; j < 16; j++)
    {
        for (uint64_t i = 0; i < 8; i++)
        {
            uint32_t nx = (x * 8 ) + i;
            uint32_t ny = (y * 16) + j;

            uint32_t color = (*face & (0b10000000 >> i)) > 0
                           ? cell->fg : cell->bg;

            *((uint32_t*)fbaddr + nx + ny * info.xres) = color;
        }
        face++;
    }  
}

void update()
{
    for (size_t i = 0; i < rows * cols; i++)
    {
        if (cells[i].dirty)
        {
            draw_cell(i % cols, i / cols);
            cells[i].dirty = 0;
        }
    }
}

void update_all()
{
    for (size_t i = 0; i < rows * cols; i++)
        draw_cell(i % cols, i / cols);
}

void clear()
{
    uint32_t* p = fbaddr;
    do *p = ansi->bg;
    while ((uintptr_t)++p < (uintptr_t)fbend);
}

void scroll_down()
{
    for (size_t y = 1; y < rows; y++)
    {
        memcpy(&cells[(y - 1) * cols], &cells[y * cols], sizeof(struct cell) * cols);
    }
    memset(&cells[(rows - 1) * cols], 0, sizeof(struct cell) * cols);
    
    update_all();
    cy = rows - 1;
}

void newline()
{
    cy++;
    cx = 0;

    if (cy == rows)
        scroll_down();
}

void tab()
{
    for (size_t i = 0; i < 4 - (cx % 4); i++)
    {
        //drawch(' ');
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
    cx--;
    setcell(cellat(cx, cy), ' ', ansi->fg, ansi->bg);
    draw_cursor();
}

void draw_cursor()
{
    //return;
    static int lx = 0, ly = 0;

    for (uint64_t j = 0; j < 16; j++)
    for (uint64_t i = 0; i < 8; i++)
    {
        uint32_t x = (cx * 8 ) + i;
        uint32_t y = (cy * 16) + j;
        *((uint32_t*)fbaddr + x + y * info.xres) = ansi->fg;
    }
    
    //setcell(cellat(lx, ly), ' ', ansi->fg, ansi->bg);
    cellat(lx, ly)->dirty = 1;
    lx = cx;
    ly = cy;
}

void putch(char c)
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

    setcell(cellat(cx, cy), c, ansi->fg, ansi->bg);

    cx++;
    if (cx == cols) newline();

    draw_cursor();
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

static const char* escapes[] =
{
    [0x48] = "\033OA", // Up
    [0x50] = "\033OB", // Down
    [0x4d] = "\033OC", // Right
    [0x4b] = "\033OD", // Left
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

    if (sc < sizeof(escapes) / sizeof(escapes[0]) && escapes[sc])
    {
        fputs(escapes[sc], ptm);
        return;
    }

    if (sc < 88)
    {
        char ch = shift ? shift_ascii[sc] : ascii[sc];

        if (ctrl)
        {
            ch = toupper(ch) - 64;
            //char c = '^';
            //fputc(c, ptm);
            //fputc(ch, ptm);
            //fprintf(ptm, "\033OA");
            fprintf(ptm, "%c", ch);
            return;
        }

        fputc(ch, ptm);
    }

}

static void clrsect(int fx, int fy, int tx, int ty)
{
    for (size_t i = fx + fy * cols; i < tx + ty * cols; i++)
    {
        memset(&cells[i], 0, sizeof(struct cell));
        cells[i].dirty = 1;
    }
    
    update();
}

static int tgetc(void) { return fgetc(ptm); }

static void gcurspos(int* x, int* y)
{
    *x = cx; *y = cy;
}

static void scurspos(int x, int y)
{
    cx = x; cy = y;
    draw_cursor();
}

struct ansicbs cbs =
{
    .gcurspos = gcurspos,
    .scurspos = scurspos,
    .tgetc    = tgetc,
    .clrsect  = clrsect
};

int main(int argc, char** argv)
{
    load_font();

    //putenv("HOME=/root");

    shift = 0;

    fb = open("/dev/fb0", O_RDWR, 0);

    ioctl(fb, FBIOGINFO, &info);

    rows = info.yres / 16;
    cols = info.xres / 8;

    size_t size = info.xres * info.yres * (info.bpp / 8);

    fbaddr = mmap(0x1000000, size, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE, fb, 0);
    fbend = (void*)((uintptr_t)fbaddr + size);

    cells = malloc(sizeof(struct cell) * rows * cols);

    char pts_name[128];
    openpty(&ptmfd, &ptsfd, pts_name, NULL, NULL);

    ptm = fdopen(ptmfd, "r+");
    pts = fdopen(ptsfd, "r+");

    struct winsize winsize =
    {
        .ws_col = cols,
        .ws_row = rows,
        .ws_xpixel = info.xres,
        .ws_ypixel = info.yres
    };

    ioctl(pts, TIOCSWINSZ, &winsize);

    ansi = ansi_init(&cbs, &winsize);
    
    dup2(ptsfd, 0);
    dup2(ptsfd, 1);
    dup2(ptsfd, 2);

    setvbuf(ptm, NULL, _IONBF, 0);
    setvbuf(pts, NULL, _IONBF, 0);

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
        char c;
        if (read(ptmfd, &c, 1))
        {
            if (c == '\033')
            {
                ansi_parse(ansi); 
            }
            else putch(c);
            update();
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
