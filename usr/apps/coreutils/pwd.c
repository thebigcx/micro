#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    char buf[64];
    getcwd(buf, 64);
    printf("%s\n", buf);
    return 0;
}