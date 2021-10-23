#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern char** environ;

int putenv(char* string)
{
    size_t i;
    for (i = 0; environ[i]; i++)
    {
        if (i >= 255) return -ENOMEM;

        size_t len1 = strcspn(string, "=");
        size_t len2 = strcspn(environ[i], "=");

        if (len1 == len2 && !strncmp(string, environ[i], len1))
        {
            environ[i] = realloc(environ[i], strlen(string) + 1);
            strcpy(environ[i], string);
            return 0;
        }
    }

    // Does not already exist
    environ[i] = strdup(string);
    environ[i + 1] = NULL;
    return 0;
}