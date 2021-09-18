#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

int closedir(DIR* dirp)
{
    close(dirp->fd);
    free(dirp);
	return 0;
}