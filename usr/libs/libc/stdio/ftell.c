#include <stdio.h>
#include <unistd.h>

long int ftell(FILE* stream)
{
    return lseek(stream->fd, 0, SEEK_CUR);        
}