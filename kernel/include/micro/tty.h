#pragma once

struct pt
{
    struct ringbuf* buffer;

    struct file*    ptm;
    struct file*    pts;
};

void tty_init();