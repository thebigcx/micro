#pragma once

#include <sys/types.h>

struct timeval
{
    time_t      tv_sec;
    suseconds_t tv_usec;
};

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

int gettimeofday(struct timeval* tv,
                 void* tz);

int utimes(const char* path, const struct timeval times[2]);