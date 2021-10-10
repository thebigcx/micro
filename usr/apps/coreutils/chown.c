#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("usage: chown [user]:[group] <file>\n");
        return -1;
    }

    uid_t uid = -1;
    gid_t gid = -1;

    char* saveptr;
    // TODO: temp
    char* token = strtok_r(argv[1], ":", &saveptr);
    
    if (token)
    {
        uid = atoi(token);
    }

    token = strtok_r(NULL, ":", &saveptr);

    if (token)
    {
        gid = atoi(token);
    }

    if (chown(argv[2], uid, gid))
    {
        perror("chown");
        return -1;
    }

    return 0;
}