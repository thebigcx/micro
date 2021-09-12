#include <time.h>
#include <assert.h>
#include <libc/sysdeps-internal.h>
#include <errno.h>

int nanosleep(const struct timespec* req, struct timespec* rem)
{
	return sys_nanosleep(req, rem);
}

time_t time(time_t* sec)
{
	time_t timer;
	int e = sys_time(&sec);

	if (e)
	{
		errno = e;
		return (time_t)-1;
	}

	if (sec) *sec = timer;
	return timer;
}

size_t strftime(char* s, size_t max,
                const char* format,
				const struct tm* tm)
{
	// TODO: implement all of these time functions

	strcpy(s, "unimpl");

	//assert(!"strftime() not implemented!\n");
	return max;
}

void localtime_r(const time_t* timer, struct tm* tm)
{

}

struct tm* localtime(const time_t* timer)
{
	static struct tm tm;
	localtime_r(timer, &tm);
	return &tm;
}

void gmtime_r(const time_t* timer, struct tm* tm)
{

}

struct tm* gmtime(const time_t* timer)
{
	static struct tm tm;
	gmtime_r(timer, &tm);
	return &tm;
}