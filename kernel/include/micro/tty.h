#pragma once

#include <micro/termios.h>

struct pt
{
    struct ringbuf* inbuf;
    struct ringbuf* outbuf;

    struct inode*   ptm;
    struct inode*   pts;

    struct winsize  size;

    struct termios  termios;

    unsigned int    num;
};

void tty_init();