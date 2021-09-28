#pragma once

#include <micro/termios.h>

struct pt
{
    struct ringbuf* inbuf;
    struct ringbuf* outbuf;

    struct file*    ptm;
    struct file*    pts;

    struct winsize  size;
};

void tty_init();