#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// TODO: follow symbolic links
char* realpath(const char* path, char* respath)
{
    char** tokens = NULL;
    size_t tokencnt = 0;
    size_t size = 0;

    char cwd[64];
    getcwd(cwd, 64);

    char* tmp = malloc(strlen(cwd) + strlen(path) + 1);
    strcpy(tmp, cwd);
    strcat(tmp, "/");
    strcat(tmp, path);

    char* saveptr;
    char* token = strtok_r(tmp + 1, "/", &saveptr);

    size = !token; // Reserve space for a '/' character

    while (token)
    {
        if (token[0] == 0 || !strcmp(token, ".")) {}
        else if (!strcmp(token, ".."))
        {
            if (tokencnt)
            {
                size -= strlen(tokens[tokencnt - 1]);
                tokencnt--;
            }
        }
        else
        {
            tokens = realloc(tokens, (tokencnt + 1) * sizeof(char*));
            tokens[tokencnt++] = token;
            size += strlen(token) + 1;
        }

        token = strtok_r(NULL, "/", &saveptr);
    }

    if (!respath) respath = calloc(1, size);

    if (!tokencnt) strcpy(respath, "/");

    for (size_t i = 0; i < tokencnt; i++)
    {
        strcat(respath, "/");
        strcat(respath, tokens[i]);
    }

    free(tmp);
    return respath;
}