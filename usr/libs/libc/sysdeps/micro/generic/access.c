#include <unistd.h>
#include <libc/syscall.h>

int access(const char* pathname, int mode)
{
	return SYSCALL_ERR(access, pathname, mode);
}