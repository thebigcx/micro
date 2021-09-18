#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>

DIR* opendir(const char* name)
{
	int fd;
	if ((fd = open(name, O_RDONLY | O_DIRECTORY, 0)) == -1) return NULL;

	DIR* dir = malloc(sizeof(DIR));
	dir->pos = 0;
	dir->fd = fd;
	return dir;
}