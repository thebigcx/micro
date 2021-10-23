#include <stdlib.h>
#include <unistd.h>

int system(const char* command)
{
    const char* argv[] = { "/bin/sh", "sh", "-c", command, NULL };
    return execv(argv[0], argv);
}