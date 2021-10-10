#include "ansi.h"

#include <stdlib.h>
#include <string.h>

// Terminators
static char terms[] =
{
    'm', 'J', 'K', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'n', 's', 'u'
};

static int isterm(char c)
{
    for (size_t i = 0; i < sizeof(terms) / sizeof(terms[0]); i++)
        if (c == terms[i]) return 1;
    return 0;
}

static uint32_t cols[] =
{
    0x00000000, // Black
    0xff800000, // Red
    0xff008000, // Green
    0xff808000, // Yellow
    0xff0000ff, // Blue
    0xff800080, // Magenta
    0xff008080, // Cyan
    0xffc0c0c0, // Light grey
    0xff808080, // Dark grey
    0xffff0000, // Light red
    0xff00ff00, // Light green
    0xffffff00, // Yellow
    0xff3d97ff, // Light blue (technically not right - VGA blue is just too dark)
    0xffff00ff, // Light magenta
    0xff00ffff, // Light cyan
    0xffffffff, // White
};

static void parse_gfx(ansi_t ansi, char* seq)
{
    char* saveptr;
    char* token = strtok_r(seq, "m;", &saveptr);
    
    while (token)
    {
        int i = atoi(token);

        if (i == 0)
        {
            ansi->fg = 0xffffffff;
            ansi->bg = 0x00000000;
        }
        else if (i >= 30 && i <= 37)
        {
            ansi->fg = cols[i - 30];
        }
        else if (i >= 40 && i <= 47)
        {
            ansi->bg = cols[i - 40];
        }
        else if (i >= 90 && i <= 97)
        {
            ansi->fg = cols[i - 90 + 8];
        }
        else if (i >= 100 && i <= 107)
        {
            ansi->bg = cols[i - 100 + 8];   
        }

        token = strtok_r(NULL, "m;", &saveptr);
    }
}

static void movecurs(ansi_t ansi, int dx, int dy)
{
    int x, y;
    ansi->cbs.gcurspos(&x, &y);
    
    if (x + dx >= 0)
        ansi->cbs.scurspos(x + dx, y);
    if (y + dy >= 0)
        ansi->cbs.scurspos(x, y + dy);
}

ansi_t ansi_init(struct ansicbs* cbs)
{
    struct ansistate* ansi = malloc(sizeof(struct ansistate));   
    
    ansi->cbs = *cbs;
    
    ansi->fg = 0xffffffff;
    ansi->bg = 0x00000000;

    return (ansi_t)ansi;
}

int ansi_parse(ansi_t ansi)
{
    ansi->cbs.tgetc(); // Skip '['

    char seq[32];
    int i = 0;
    while (!isterm(seq[i++] = ansi->cbs.tgetc()));
    seq[i] = 0;

    switch (seq[i - 1])
    {
        case 'm':
            parse_gfx(ansi, seq);
            return 0;
        case 'D':
        {
            seq[i - 1] = 0;
            int n = i - 1 == 0 ? 1 : atoi(seq); // Can omit # of cols
            movecurs(ansi, -n, 0);
            return 0;
        }
    }

    return 0;
}
