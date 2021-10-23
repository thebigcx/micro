#include <unistd.h>
#include <sys/syscall.h>

pid_t getpid()
{
	return syscall(SYS_getpid);
}