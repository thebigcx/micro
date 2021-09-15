#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: ld.so <executable>\n");
        return -1;
    }

    return 0;
}