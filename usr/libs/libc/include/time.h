#pragma once

#include <stddef.h>

typedef long int time_t;

struct timespec
{
	time_t tv_sec;  // Seconds
	long   tv_nsec; // Nanoseconds
};

struct tm
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;  // Day of the month
	int tm_mon;
	int tm_year;
	int tm_wday;  // Week day (since Sunday)
	int tm_yday;  // Year day (0 - 365)
	int tm_isdst; // Daylight Savings Time flag
};

typedef long int clock_t;

#define CLOCKS_PER_SEC ((clock_t)1000000)

int nanosleep(const struct timespec* req, struct timespec* rem);
time_t time(time_t* sec);

size_t strftime(char* s, size_t max,
                const char* format,
				const struct tm* tm);

struct tm* localtime(const time_t* timer);
struct tm* gmtime(const time_t* timer);

char* ctime(const time_t* timer);
char* ctime_r(const time_t* timer, char* buf);

char* asctime(const struct tm* timeptr);
char* asctime_r(const struct tm* timeptr, char* buf);

double difftime(time_t t1, time_t t2);

time_t mktime(struct tm* tm);

clock_t clock();