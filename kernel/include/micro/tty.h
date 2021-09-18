#pragma once

struct pt
{
    struct ringbuf* inbuf;
    struct ringbuf* outbuf;

    struct file*    ptm;
    struct file*    pts;
};

void tty_init();