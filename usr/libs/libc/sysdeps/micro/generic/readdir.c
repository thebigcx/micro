#include <dirent.h>
#include <stdlib.h>
#include <libc/syscall.h>

struct dirent* readdir(DIR* dirp)
{
	struct dirent* dirent = malloc(sizeof(struct dirent));
	//int e = sys_readdir(dirp->fd, dirp->pos++, dirent);
	ssize_t bytes;
	int e = sys_getdents(dirp->fd, dirent, 1, &bytes);

	if (e == -1 || bytes == 0) return NULL;

	return dirent;
}