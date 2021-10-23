#pragma once

#include <sys/types.h>

#define RLIM_INFINITY   ((rlim_t)-1)

#define RLIMIT_NOFILE   0

struct rlimit
{
    rlim_t rlim_cur;
    rlim_t rlim_max;
};

int getrlimit(int resource, struct rlimit* rlim);
int setrlimit(int resource, const struct rlimit* rlim);