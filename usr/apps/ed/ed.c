#include <stdio.h>
#include <stdint.h>

static FILE* file;
static uintptr_t linenr;

void do_insert()
{
    printf("insert: ");

    char* insert = malloc(256);
    fgets(insert, 256, stdin);

    char* ptr = strchr(insert, '\n');
    if (ptr) *ptr = 0;

    size_t old = ftell(file);
    fwrite(insert, strlen(insert), 1, file);
    fseek(file, old, SEEK_SET);
}

void do_command(char* cmd)
{
    if (!strcmp(cmd, "i") || !strcmp(cmd, "insert"))
    {
        do_insert();
    }
    else if (!strcmp(cmd, "q") || !strcmp(cmd, "quit"))
    {
        fclose(file);
        exit(0);
    }
    else if (!strcmp(cmd, "f") || !strcmp(cmd, "file"))
    {
        size_t old = ftell(file);

        fseek(file, 0, SEEK_END);
        size_t len = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* data = malloc(len);
        fread(data, len, 1, file);

        fseek(file, old, SEEK_SET);

        for (size_t i = 0; i < len; i++) printf("%c", data[i]);
        printf("\n");

        free(data);
    }
    else if (!strcmp(cmd, "p") || !strcmp(cmd, "print"))
    {
        char* line = malloc(256);
        fgets(line, 256, file);

        printf("%s", line);

        free(line);
    }
    else if (!strcmp(cmd, "n") || !strcmp(cmd, "number"))
    {
        printf("%d\n", linenr + 1);
    }
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: ed [filename]\n");
        return 0;
    }

    file = fopen(argv[1], "r");
    linenr = 0;

    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = malloc(len);
    fread(data, len, 1, file);

    fclose(file);

    file = fopen(argv[1], "w+");
    fwrite(data, len, 1, file);
    fseek(file, 0, SEEK_SET);

    char* line = malloc(256);

    for (;;)
    {
        printf("(ed) ");

        fgets(line, 256, stdin);
        char* ptr = strchr(line, '\n');
        if (ptr) *ptr = 0;
        
        do_command(line);
    }

    free(line);
    fclose(file);

    return 0;
}
