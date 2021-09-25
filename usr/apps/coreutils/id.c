#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* passwd;
static size_t passwd_size;

static char* group;
static size_t group_size;

void get_username(uid_t uid, char* out)
{
    char* line = passwd;
    while (1)
    {
        size_t len = strcspn(line, "\n");

        char* linecpy = malloc(len + 1);
        strcpy(linecpy, line);

        line += len + 1;

        char* saveptr;
        char* name = strtok_r(linecpy, ":", &saveptr);
        char* uidstr = strtok_r(NULL, ":", &saveptr);

        if (atoi(uidstr) == uid)
        {
            strcpy(out, name);
            free(linecpy);
            return;
        }

        free(linecpy);

        if (line >= passwd + passwd_size) break;
    }
}

char* get_groupname(gid_t gid, char* out)
{
    char* line = group;
    while (1)
    {
        size_t len = strcspn(line, "\n");

        char* linecpy = malloc(len + 1);
        strcpy(linecpy, line);

        line += len + 1;

        char* saveptr;
        char* name = strtok_r(linecpy, ":", &saveptr);
        char* gidstr = strtok_r(NULL, ":", &saveptr);

        if (atoi(gidstr) == gid)
        {
            strcpy(out, name);
            free(linecpy);
            return;
        }

        free(linecpy);

        if (line >= group + group_size) break;
    }
}

void readfile(const char* path, char** data, size_t* size)
{
    FILE* file = fopen(path, "r");
    
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *data = malloc(*size);
    fread(*data, *size, 1, file);

    fclose(file);
}

int main(int argc, char** argv)
{
    readfile("/etc/passwd", &passwd, &passwd_size);
    readfile("/etc/group", &group, &group_size);

    uid_t uid = geteuid();
    gid_t gid = getegid();

    gid_t gids[64];
    int gidcnt = getgroups(64, gids);

    char username[64];
    get_username(uid, username);

    char groupname[64];
    get_groupname(gid, groupname);

    printf("uid=%d(%s) gid=%d(%s) groups=%d(%s)", uid, username, gid, groupname, gid, groupname);
    
    for (int i = 0; i < gidcnt; i++)
    {
        printf(",%d", gids[i]);
    }

    printf("\n");

    return 0;
}