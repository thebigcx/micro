#include <unistd.h>
#include <libc/syscall.h>

off_t lseek(int fd, off_t off, int whence)
{
	return SYSCALL_ERR(lseek, fd, off, whence);
}