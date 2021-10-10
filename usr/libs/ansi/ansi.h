#pragma once

#include <stdint.h>

// ANSI callback functions
struct ansicbs
{
    void (*scurspos)(int, int);
    void (*gcurspos)(int*, int*);

    int (*tgetc)(void); // Terminal get char
};

struct ansistate
{
    struct ansicbs cbs;

    uint32_t fg, bg;
};

typedef struct ansistate* ansi_t;

ansi_t ansi_init(struct ansicbs* cbs);
int ansi_parse(ansi_t ansi);
