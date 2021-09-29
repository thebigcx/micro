#include <sys/utsname.h>
#include <stdio.h>
#include <unistd.h>

int all = 0;
int name = 0;
int nodename = 0;
int release = 0;
int version = 0;
int machine = 0;

int main(int argc, char** argv)
{
    char opt;
    while ((opt = getopt(argc, argv, "asnrvm")) != -1)
    {
        switch (opt)
        {
            case 'a': all = 1; break;
            case 's': name = 1; break;
            case 'n': nodename = 1; break;
            case 'r': release = 1; break;
            case 'v': version = 1; break;
            case 'm': machine = 1; break;
            default: return -1;
        }
    }

    struct utsname buf;
    uname(&buf);

    if (!all && !name && !nodename && !release && !version && !machine) name = 1;

    if (name || all) printf("%s ", buf.sysname);
    if (nodename || all) printf("%s ", buf.nodename);
    if (release || all) printf("%s ", buf.release);
    if (version || all) printf("%s ", buf.version);
    if (machine || all) printf("%s ", buf.machine);

    printf("\n");

    return 0;
}