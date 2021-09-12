#include <unistd.h>
#include <dirent.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <libc/sysdeps-internal.h>

int open(const char* filename, int flags, mode_t mode)
{
	int fd;
	int err = sys_open(filename, flags, mode, &fd);

	if (err)
	{
        errno = err;
		return -1;
	}

	return fd;
}

int close(int fd)
{
	int err = sys_close(fd);
	
	if (err)
	{
        errno = err;
		return -1;
	}

	return 0;
}

ssize_t read(int fd, void* buf, size_t count)
{
	ssize_t bytes_read;
	int err = sys_read(fd, buf, count, &bytes_read);

	if (err)
	{
        errno = err;
		return -1;
	}

	return bytes_read;
}

ssize_t write(int fd, const void* buf, size_t count)
{
	ssize_t bytes_written;
	int err = sys_write(fd, buf, count, &bytes_written);

	if (err)
	{
        errno = err;
		return -1;
	}

	return bytes_written;
}

int access(const char* pathname, int mode)
{
	int err = sys_access(pathname, mode);
	if (err)
	{
		errno = err;
		return -1;
	}

	return 0;
}

off_t lseek(int fd, off_t off, int whence)
{
    off_t npos;
    int err = sys_lseek(fd, off, whence, &npos);
    
    if (err)
    {
        errno = err;
        return -1;
    }

    return npos;
}

int ioctl(int fd, unsigned long request, void* argp)
{
	int ret = sys_ioctl(fd, request, argp);
	if (ret < 0)
	{
		errno = ret;
		return -1;
	}

	return ret;
}

// TODO: make this better
struct dirent* readdir(DIR* dirp)
{
	struct dirent* dirent = malloc(sizeof(struct dirent));
	int e = sys_readdir(dirp->fd, dirp->pos++, dirent);

	if (e == 1)
		return dirent;
	
	return NULL;
}

DIR* opendir(const char* name)
{
	DIR* dir = malloc(sizeof(DIR));
	dir->pos = 0;
	dir->fd = open(name, 0, 0);
	return dir;
}

int closedir(DIR* dirp)
{
	return -1;
}

int mkdir(const char* path, mode_t mode)
{
	int e = sys_mkdir(path, mode);

	if (!e) return 0;

	errno = e;
	return -1;
}

int truncate(const char* path, off_t len)
{
	// TODO: truncate syscall
	return 0;
}

int ftruncate(int fd, off_t len)
{
	// TODO: ftruncate syscall
	return 0;
}

int isatty(int fd)
{
	// TODO
	return 1;
}

int dup(int oldfd)
{
	int e = sys_dup(oldfd);
	
	if (e)
	{
		errno = e;
		return -1;
	}

	return 0;
}

int dup2(int oldfd, int newfd)
{
	int e = sys_dup2(oldfd, newfd);
	
	if (e)
	{
		errno = e;
		return -1;
	}

	return 0;
}