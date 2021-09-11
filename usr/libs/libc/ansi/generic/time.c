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
	assert(!"strftime() not implemented!\n");
	return 0;
}

struct tm* localtime(const time_t* timer)
{
	assert(!"localtime() not implemented!\n");
	return NULL;
}

struct tm* gmtime(const time_t* timer)
{
	assert(!"gmtime() not implemented!\n");
	return NULL;
}