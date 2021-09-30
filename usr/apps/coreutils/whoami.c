#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int compar(const void* a, const void* b)
{
    printf("%p, %p\n", a, b);
    return *((char**)a)[0] - *((char**)b)[0];
}

int main(int argc, char** argv)
{
    // TEST
    /*int fds[2];
    pipe(fds);

    if (fork() == 0)
    {
        dup2(fds[1], STDOUT_FILENO);
        
        const char* argv[] = { "/usr/bin/tee", NULL };
        execv(argv[0], argv);
    }
    else
    {
        dup2(fds[0], STDIN_FILENO);
        
        const char* argv[] = { "/usr/bin/sh", NULL };
        execv(argv[0], argv);
    }*/
    //*((uint32_t*)0x89735097377207) = 1000;

    const char* names[] = { "Chris", "John", "Alex", "Connor", "Ethan", "Lucas", "George", "Noah" };
    qsort(names, 8, sizeof(const char*), compar);

    for (size_t i = 0; i < 8; i++)
        printf("%s, ", names[i]);


    uid_t uid = geteuid();

    FILE* passwd = fopen("/etc/passwd", "r");

    while (1)
    {
        char line[256];
        size_t linesz = 0;
        int eof = 0;
        while (linesz < 256)
        {
            ssize_t r = fread(&line[linesz++], 1, 1, passwd);
            if (r == EOF) { eof = 1; break; }
            else if (line[linesz - 1] == '\n') { break; }
        }
        line[linesz] = 0;

        char* saveptr;
        char* user = strtok_r(line, ":", &saveptr);
        char* userid = strtok_r(NULL, ":", &saveptr);

        if (atoi(userid) == uid)
        {
            printf("%s\n", user);
            fclose(passwd);
            return 0;
        }

        if (eof) break;
    }

    fclose(passwd);
    printf("invalid user\n");
    return -1;
}