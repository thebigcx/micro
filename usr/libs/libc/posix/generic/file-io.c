#include <unistd.h>
#include <dirent.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libc/sysdeps-internal.h>

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