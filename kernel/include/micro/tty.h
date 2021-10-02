#pragma once

#include <micro/termios.h>

struct pt
{
    struct ringbuf* inbuf;
    struct ringbuf* outbuf;

    struct inode*    ptm;
    struct inode*    pts;

    struct winsize  size;
};

void tty_init();