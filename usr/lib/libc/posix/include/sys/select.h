#pragma once

typedef struct
{
    unsigned long fds[1024 / 64];
} fd_set;