#pragma once

#include <micro/termios.h>
#include <micro/types.h>

struct pt
{
    struct ringbuf* inbuf;
    struct ringbuf* outbuf;

    struct inode*   ptm;
    struct inode*   pts;

    struct winsize  size;

    struct termios  termios;

    unsigned int    num;
    pid_t           pgid;
};

void tty_init();
