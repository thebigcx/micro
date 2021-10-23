#include <dirent.h>
#include <stdlib.h>
#include <libc/syscall.h>

struct dirent* readdir(DIR* dirp)
{
	struct dirent* dirent = malloc(sizeof(struct dirent));
    
	ssize_t bytes = syscall(SYS_getdents, dirp->fd, dirent, 1);
	if (bytes <= 0) return NULL;

	return dirent;
}