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

int nanosleep(const struct timespec* req, struct timespec* rem);
time_t time(time_t* sec);

size_t strftime(char* s, size_t max,
                const char* format,
				const struct tm* tm);

struct tm* localtime(const time_t* timer);
struct tm* gmtime(const time_t* timer);