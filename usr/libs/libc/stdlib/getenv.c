#include <stdlib.h>

extern char** environ;

char* getenv(const char* name)
{
    char** env = environ;

    while (*env != NULL)
    {
        size_t len = strlen(name);
        char* equ = strchr(*env, '=');

        if (!strncmp(*env, name, len)
          && equ - *env == len)
        {
            return equ + 1;
        }

        env++;
    }

    return NULL;
}