#pragma once

typedef long int time_t;
typedef long int suseconds_t;

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

struct utimbuf
{
    time_t actime;  // Access
    time_t modtime; // Modification
};

struct timespec
{
    time_t   tv_sec;
    long int tv_nsec;
};
