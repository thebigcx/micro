#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

int main(int argc, char** argv)
{
    struct passwd* passwd = getpwuid(getuid());

    // TODO: groups
    printf("%s(%d)\n", passwd->pw_name, passwd->pw_uid);

    return 0;
}
