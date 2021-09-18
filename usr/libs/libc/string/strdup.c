#include <string.h>

char* strdup(const char* s)
{
    char* news = malloc(strlen(s) + 1);
    strcpy(news, s);
    return news;
}