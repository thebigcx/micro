#include <unistd.h>
#include <libc/sysdeps-internal.h>
#include <libc/libc-internal.h>
#include <errno.h>

pid_t getpid()
{
	return sys_getpid();
}

pid_t fork()
{
	pid_t pid;
	int e = sys_fork(&pid);
	if (e != 0)
	{
		errno = e;
        return -1;
	}

	return pid;
}

int execve(const char* pathname, const char* argv[], const char* envp[])
{
	int e = sys_execve(pathname, argv, envp);
	errno = e;
    return -1;
}

int execv(const char* path, const char* argv[])
{
    return execve(path, argv, environ); 
}

int execvp(const char* file, const char* argv[])
{
    // TODO: parse the PATH environment variable
    assert(!"execvp() not implemented!\n");
    return -1;
}

pid_t wait(int* status)
{
	pid_t pid;
	int e = sys_wait(status, &pid);
	
	if (e != 0)
	{
		errno = e;
		return -1;
	}

	return pid;
}

int chdir(const char* path)
{
	int e = sys_chdir(path);

	if (e != 0)
	{
		errno = e;
		return -1;
	}

	return 0;
}

char* getcwd(char* buf, size_t size)
{
	char* ret;
	int e = sys_getcwd(buf, size, &ret);

	if (e != 0)
	{
		errno = e;
		return (char*)-1;
	}

	return ret;
}
