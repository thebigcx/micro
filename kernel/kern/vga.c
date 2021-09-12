#include <micro/vga.h>

#define COLS 80
#define ROWS 25

static struct vga_char* buf;
static unsigned int cx = 0, cy = 0;
static unsigned int col = (VGA_BLACK << 4) | VGA_WHITE;

struct __attribute__((packed)) vga_char
{
    uint8_t c;
    uint8_t col;
};

void vga_set_addr(uintptr_t addr)
{
    buf = (struct vga_char*)addr;
}

void clear_row(unsigned int row)
{
    struct vga_char ch =
    {
        .c = ' ',
        .col = col
    };

    for (unsigned int x = 0; x < COLS; x++)
    {
        buf[x + row * COLS] = ch;
    }
}

void newline()
{
    cx = 0;
    
    if (cy < ROWS - 1)
    {
        cy++;
        return;
    }

    for (unsigned int y = 1; y < ROWS; y++)
    for (unsigned int x = 0; x < COLS; x++)
    {
        buf[x + (y - 1) * COLS] = buf[x + y * COLS];
    }

    clear_row(ROWS - 1);
}

void backspace()
{
    if (cx == 0)
    {
        cx = ROWS - 1;
        cy--; // TODO: scroll up if need be
    }
    cx--;

    buf[cx + cy * COLS] = (struct vga_char)
    {
        .c = ' ',
        .col = col
    };
}

void vga_putc(char c)
{
    if (c == '\n')
    {
        newline();
        return;
    }
    if (c == '\b')
    {
        backspace();
        return;
    }

    struct vga_char ch =
    {
        .c = (uint8_t)c,
        .col = col
    };

    buf[cx + cy * COLS] = ch;

    cx++;
    if (cx == COLS)
    {
        newline();
    }
}

void vga_set_color(int fg, int bg)
{
    col = (bg << 4) | fg;
}