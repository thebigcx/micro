#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: fdisk <device>\n");
        return -1;
    }
    
    return 0;
}