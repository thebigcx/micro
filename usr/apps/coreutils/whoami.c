#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
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