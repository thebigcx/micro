#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

FILE* tmpfile()
{
    /*FILE* file = malloc(sizeof(FILE));

    file->fd = open("/tmp", O_RDWR | O_TMPFILE, 0);
    file->err = 0;
    file->eof = 0;

    return file;*/

    const char* prefix = "/tmp/";

    char* path = malloc(strlen(prefix) + 2);
    memcpy(path, prefix, strlen(prefix));
    path[strlen(prefix) + 1] = 0;
    char c = 'a';

    while (1)
    {
        path[strlen(prefix)] = c++;
        if (!access(path, F_OK))
        {
            FILE* file = fopen(path, "w+");
            free(path);
            return file;
        }
    }

    return NULL;
}