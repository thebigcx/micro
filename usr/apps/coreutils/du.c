#include <stdio.h>

#define GB 1000000000
#define MB 1000000
#define KB 1000
#define B  1

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: du <filename>\n");
        return -1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file)
    {
        perror("du: ");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);

    printf("%s: ", argv[1]);

    if (size >= GB)
        printf("%ld GB", size / GB);
    else if (size >= 1000000)
        printf("%ld MB", size / MB);
    else if (size >= 1000)
        printf("%ld KB", size / KB);
    else
        printf("%ld B", size / B);

    printf("\n");

    fclose(file);
    return 0;
}