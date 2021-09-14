#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    char buf[64];
    if (getcwd(buf, 64) == -1)
    {
        perror("pwd: ");
        return -1;
    }

    printf("%s\n", buf);
    return 0;
}