#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: hexdump <filename>\n");
        return 0;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file)
    {
        perror("hexdump: ");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* buffer = malloc(size);
    fread(buffer, size, 1, file);

    printf("0x0 ");

    for (size_t i = 0; i < size; i++)
    {
        if (buffer[i] < 16) // TODO: padding in printf()
            printf("0");

        printf("%x", buffer[i]);

        if ((i + 1) % 16 == 0)
        {
            printf("\n");
            printf("0x%x ", i);
            continue;
        }

        if ((i + 1) % 2 == 0)
            printf(" ");
    }

    printf("\n");

    fclose(file);
    free(buffer);
    return 0;
}