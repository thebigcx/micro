#pragma once

#include <stdint.h>
#include <termios.h>

// ANSI callback functions
struct ansicbs
{
    void (*scurspos)(int, int);
    void (*gcurspos)(int*, int*);

    int (*tgetc)(void); // Terminal get char

    void (*clrsect)(int, int, int, int); // Clear from from-to
};

struct ansistate
{
    struct ansicbs cbs;

    uint32_t fg, bg;
    struct winsize sz;
};

typedef struct ansistate* ansi_t;

ansi_t ansi_init(struct ansicbs* cbs, struct winsize* sz);
int ansi_parse(ansi_t ansi);
