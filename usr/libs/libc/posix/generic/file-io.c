#include <unistd.h>
#include <dirent.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
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
	ssize_t bytes;
	int err = sys_read(fd, buf, count, &bytes);

	if (err)
	{
        errno = err;
		return -1;
	}

	return bytes;
}

ssize_t write(int fd, const void* buf, size_t count)
{
	ssize_t bytes;
	int err = sys_write(fd, buf, count, &bytes);

	if (err)
	{
        errno = err;
		return -1;
	}

	return bytes;
}

ssize_t pread(int fd, void* buf, size_t count, off_t off)
{
	ssize_t bytes;
	int err = sys_pread(fd, buf, count, off, &bytes);

	if (err)
	{
        errno = err;
		return -1;
	}

	return bytes;
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t off)
{
	ssize_t bytes;
	int err = sys_pwrite(fd, buf, count, off, &bytes);

	if (err)
	{
        errno = err;
		return -1;
	}

	return bytes;
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
	//int e = sys_readdir(dirp->fd, dirp->pos++, dirent);
	ssize_t bytes;
	int e = sys_getdents(dirp->fd, dirent, 1, &bytes);

	if (e == -1 || bytes == 0) return NULL;

	return dirent;
}

DIR* opendir(const char* name)
{
	int fd;
	if ((fd = open(name, O_RDONLY | O_DIRECTORY, 0)) == -1) return NULL;

	DIR* dir = malloc(sizeof(DIR));
	dir->pos = 0;
	dir->fd = fd;
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