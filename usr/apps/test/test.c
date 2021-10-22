#include <stdio.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
    int fds[2];
    pipe(fds);

    if (fork() == 0)
    {
        open(fds[0], O_WRONLY);
        char* const argv[] = { "/usr/bin/ls", NULL };
        execv(argv[0], argv);
    }
    else
    {
        open(fds[1], O_RDONLY);
        char* const argv[] = { "/usr/bin/grep", "bin", NULL };
        execv(argv[0], argv);
    }

    return 0;
}
