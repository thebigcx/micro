#include <stdio.h>

int main(int argc, char** argv)
{
    FILE* file = argv[1] ? fopen(argv[1], "w+") : stderr;
    if (!file)
    {
        perror("tee");
        return -1;
    }

    char c;
    while ((c = fgetc(stdin)) != EOF)
    {
        fprintf(file, "%c", c);
        printf("%c", c);
    }
    
    return 0;
}