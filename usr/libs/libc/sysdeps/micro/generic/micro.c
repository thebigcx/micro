#include <libc/sysdeps-internal.h>
#include <micro/syscall.h>
#include <errno.h>

int sys_open(const char* pathname, int flags, mode_t mode, int* fd)
{
	long ret = syscall(SYS_open, pathname, flags, mode);
	
	*fd = ret;

	if (ret < 0)
		return ret;

	return 0;
}

int sys_close(int fd)
{
	return syscall(SYS_close, fd);
}

int sys_read(int fd, void* buf, size_t cnt, ssize_t* bytes_read)
{
	ssize_t ret = syscall(SYS_read, fd, buf, cnt);

	if (ret < 0)
		return ret;

	*bytes_read = ret;
	return 0;
}

int sys_write(int fd, const void* buf, size_t cnt, ssize_t* bytes_written)
{
	ssize_t ret = syscall(SYS_write, fd, buf, cnt);

	if (ret < 0)
        return ret;

	*bytes_written = ret;
	return 0;
}

int sys_fork(pid_t* pid)
{
	pid_t ret = syscall(SYS_fork);
	if (ret < 0)
	{
		return -1;
	}

	*pid = ret;
	return 0;
}

int sys_execve(const char* pathname, const char* argv[], const char* envp[])
{
	int e = syscall(SYS_execve, pathname, argv, envp);
	return e;
}

int sys_exit(int status)
{
	syscall(SYS_exit, status);
	return 0;
}

int sys_kill(pid_t pid, int sig)
{
	return syscall(SYS_kill, pid, sig);
}

pid_t sys_getpid()
{
	return (pid_t)syscall(SYS_getpid);
}

int sys_access(const char* pathname, int mode)
{
	return syscall(SYS_access, pathname, mode);
}

int sys_lseek(int fd, off_t off, int whence, off_t* noff)
{
    off_t ret = syscall(SYS_lseek, fd, off, whence);
    
    *noff = ret;
    
    if (ret < 0)
        return ret;

    return 0;
}

int sys_waitpid(pid_t pid, int* wstatus, int options, pid_t* ret)
{
	pid_t res = syscall(SYS_waitpid, pid, wstatus, options);
	if (res < 0)
	{
		return res;
	}

	*ret = res;
	return 0;
}

int sys_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset, void** ret)
{
    void* mapped_addr = (void*)syscall(SYS_mmap, addr, length, prot, flags, fd, offset);

	// TODO: error checking

    *ret = mapped_addr;

    return 0;
}

int sys_munmap(void* addr, size_t length)
{
    return syscall(SYS_munmap, addr, length);
}

int sys_chdir(const char* path)
{
	return syscall(SYS_chdir, path);
}

int sys_getcwd(char* buf, size_t size, char** ret)
{
	char* cwd = (char*)syscall(SYS_getcwd, buf, size);

	// TODO: don't do this
	if (!cwd)
		return -1;
	
	*ret = cwd;
	return 0;
}

int sys_readdir(int fd, size_t idx, struct dirent* dirent)
{
	int e = syscall(SYS_readdir, fd, idx, dirent);

	// TODO: don't do this
	if (e < 0)
	{
		errno = e;
		return -1;
	}

	return e;
}

int sys_mkdir(const char* path, mode_t mode)
{
	return syscall(SYS_mkdir, path, mode);
}

int sys_ioctl(int fd, unsigned long request, void* argp)
{
	return syscall(SYS_ioctl, fd, request, argp);
}

int sys_nanosleep(const struct timespec* req, struct timespec* rem)
{
	// TODO
	return 0;
}

int sys_time(time_t* timer)
{
	return syscall(SYS_time, timer);
}

int sys_dup(int oldfd)
{
	return syscall(SYS_dup);
}

int sys_dup2(int oldfd, int newfd)
{
	return syscall(SYS_dup2);
}