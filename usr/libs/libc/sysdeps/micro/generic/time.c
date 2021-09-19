#include <time.h>
#include <libc/syscall.h>

time_t time(time_t* sec)
{
	time_t timer;
    long e = SYSCALL_ERR(time, &timer);

	if (e)
		return (time_t)-1;

	if (sec) *sec = timer;
	return timer;
}