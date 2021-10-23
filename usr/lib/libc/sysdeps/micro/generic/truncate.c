#include <unistd.h>
#include <libc/syscall.h>

int truncate(const char* path, off_t len)
{
	// TODO: truncate syscall
	return 0;
}