#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdio.h>
#include <ansi.h>
#include <sys/ioctl.h>

// TODO: put in <micro/vga.h>
#define VGASCURSPOS 0

struct cursor
{
    uint8_t x, y;  
};

struct __attribute__((packed)) vga_char
{
    uint8_t c;
    uint8_t col;
};

enum VGA_COLS
{
    VGA_BLACK,
    VGA_BLUE,
	VGA_GREEN,
	VGA_CYAN,
	VGA_RED,
	VGA_MAGENTA,
	VGA_BROWN,
	VGA_LGREY,
	VGA_DGREY,
	VGA_LBLUE,
	VGA_LGREEN,
	VGA_LCYAN,
	VGA_LRED,
	VGA_PINK,
	VGA_YELLOW,
	VGA_WHITE
};

// ANSI to VGA
static uint32_t cols[] =
{
    VGA_BLACK,
    VGA_RED,
    VGA_GREEN,
    VGA_BROWN,
    VGA_BLUE,
    VGA_MAGENTA,
    VGA_CYAN,
    VGA_LGREY,
    VGA_DGREY,
    VGA_LRED,
    VGA_LGREEN,
    VGA_YELLOW,
    VGA_LBLUE,
    VGA_PINK,
    VGA_LCYAN,
    VGA_WHITE
};

static int ptsfd, ptmfd, vgafd;
static FILE* ptm, *pts;

static int width, height;

static unsigned int cx = 0, cy = 0;
static struct vga_char* buffer;

static ansi_t ansi;

void scroll()
{
    cy = height - 1;
    
    for (size_t i = 0; i < height - 1; i++)
        memcpy(buffer + i * width, buffer + (i + 1) * width, sizeof(struct vga_char) * width);
    
    memset(buffer + (height - 1) * width, 0, sizeof(struct vga_char) * width);
}

void newline()
{
    cx = 0;
    cy++;
    
    if (cy == height)
        scroll();
}

void clear()
{
    memset(buffer, 0, width * height * sizeof(struct vga_char));   
}

void backspace()
{
    cx--;
    putch(' ');
    cx--;
}

void putch(char ch)
{
    if (ch == '\n')
    {
        newline();
        return;
    }
    if (ch == '\b')
    {
        backspace();
        return;
    }

    buffer[cy * width + cx] = (struct vga_char)
    {
        .c = ch,
        .col = (cols[ansi->bg] << 4) | cols[ansi->fg]
    };
    
    cx++;
    if (cx == width)
        newline();
}

void update_curs()
{
    struct cursor pos = { .x = cx, .y = cy };
    ioctl(vgafd, VGASCURSPOS, &pos);   
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
            return;
        }

        fputc(ch, ptm);
    }

}

void gcurspos(int* x, int* y) { *x = cx; *y = cy; }
void scurspos(int x, int y) { cx = x; cy = y; }

int tgetc() { return fgetc(ptm); }

void clrsect(int fx, int fy, int tx, int ty)
{
    memset(buffer + ((fy * width) + fx), 0, (ty * width + tx) * sizeof(struct vga_char));
}

struct ansicbs cbs =
{
    .gcurspos = gcurspos,
    .scurspos = scurspos,
    .tgetc    = tgetc,
    .clrsect  = clrsect
};

int main()
{
    vgafd = open("/dev/vga0", O_RDWR, 0);

    // TODO: use an ioctl
    width = 80; height = 25;

    uintptr_t pages = width * height * sizeof(struct vga_char);
    pages += (0x1000 - pages % 0x1000);

    buffer = mmap(0, pages, PROT_READ | PROT_WRITE, MAP_PRIVATE, vgafd, 0);

    openpty(&ptmfd, &ptsfd, NULL, NULL, NULL);
   
    ptm = fdopen(ptmfd, "r+");
    pts = fdopen(ptsfd, "r+");

    dup2(ptsfd, 0);
    dup2(ptsfd, 1);
    dup2(ptsfd, 2);

    struct winsize winsize =
    {
        .ws_col = width,
        .ws_row = height,
        .ws_xpixel = width * 8,
        .ws_ypixel = height * 16
    };

    ioctl(ptsfd, TIOCSWINSZ, &winsize);

    ansi = ansi_init(&cbs, &winsize);

    //clear();

    setvbuf(ptm, NULL, _IONBF, 0);
    setvbuf(pts, NULL, _IONBF, 0);

    int pid = fork();
    if (pid == 0)
    {
        const char* argv[] = { "/usr/bin/bash", NULL };
        const char* envp[] = { NULL };
        execve(argv[0], argv, envp);
        for (;;);
    }
    
    int kb = open("/dev/keyboard", O_RDONLY);

    for (;;)
    {
        char c;
        if (read(ptmfd, &c, 1))
        {
            if (c == '\033')
                ansi_parse(ansi);
            else
                putch(c);   
            update_curs();
        }

        int sc;
        if (read(kb, &sc, 1))
            handle_kb(sc);

        int status;
        if (waitpid(pid, &status, WNOHANG) > 0)
            break;
    }

    return 0;
}
