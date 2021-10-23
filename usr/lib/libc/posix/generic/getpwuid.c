#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static FILE* passwd = NULL;
static struct passwd passwds[128];
static size_t passwds_cnt = 0;

static void __read_passwd()
{
    passwd = fopen("/etc/passwd", "r");
        
    char line[256];
    while (fgets(line, 256, passwd))
    {
        char* saveptr;

        struct passwd p;

        // TODO: permit empty entries
        char* name   = strtok_r(line, ":", &saveptr);
        char* passwd = strtok_r(NULL, ":", &saveptr);
        char* uid    = strtok_r(NULL, ":", &saveptr);
        char* gid    = strtok_r(NULL, ":", &saveptr);
        char* gecos  = strtok_r(NULL, ":", &saveptr);
        char* dir    = strtok_r(NULL, ":", &saveptr);
        char* shell  = strtok_r(NULL, ":\n", &saveptr);

        p.pw_name   = name   ? strdup(name)   : NULL;
        p.pw_passwd = passwd ? strdup(passwd) : NULL;
        p.pw_uid    = uid    ? atoi(uid)      : 0;
        p.pw_gid    = gid    ? atoi(gid)      : 0;
        p.pw_gecos  = gecos  ? strdup(gecos)  : NULL;
        p.pw_dir    = dir    ? strdup(dir)    : NULL;
        p.pw_shell  = shell  ? strdup(shell)  : NULL;

        passwds[passwds_cnt++] = p;
    }
}

struct passwd* getpwuid(uid_t uid)
{
    if (!passwd)
        __read_passwd();

    for (size_t i = 0; i < passwds_cnt; i++)
    {
        if (passwds[i].pw_uid == uid)
            return &passwds[i];
    }

    return NULL;
}

struct passwd* getpwnam(const char* name)
{
    if (!passwd)
        __read_passwd();

    for (size_t i = 0; i < passwds_cnt; i++)
    {
        if (!strcmp(passwds[i].pw_name, name))
            return &passwds[i];
    }

    return NULL;
}