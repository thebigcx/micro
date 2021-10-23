#include <time.h>

struct tm* gmtime(const time_t* timer)
{
	static struct tm tm;
	gmtime_r(timer, &tm);
	return &tm;
}