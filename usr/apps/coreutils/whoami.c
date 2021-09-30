#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>

int main(int argc, char** argv)
{
    struct passwd* p = getpwuid(1000);
    if (!p)
    {
        printf("whoami: invalid user\n");
        return -1;
    }

    printf("%s\n", p->pw_name);
    return 0;
}