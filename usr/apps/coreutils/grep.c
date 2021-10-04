#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

void usage()
{
    printf("usage: grep <regex> <filename>\n");
}

int main(int argc, char** argv)
{
    struct sigaction act;
    //memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);

    char* buffer = malloc(1024);
    size_t size;
    if (argc == 3)
    {
        FILE* file = fopen(argv[2], "r");
        if (!file)
        {
            perror("grep: ");
            return -1;
        }
        
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);

        fread(buffer, size, 1, file);
    }
    else if (argc == 2)
    {
        char c;
        size_t i;
        for (i = 0; (c = fgetc(stdin)) != EOF; i++)
            buffer[i] = c;

        size = i;
    }
    else
    {
        usage();
        return -1;
    }

    char* pattern = argv[1];

    // TODO: regex instead of silly search
    char* saveptr;
    char* line = strtok_r(buffer, "\n", &saveptr);

    while (line)
    {
        for (size_t idx = 0; line[idx]; idx++)
        {
            if (!strncmp(pattern, line + idx, strlen(pattern)))
                printf("%s\n", line);
        }

        line = strtok_r(NULL, "\n", &saveptr);
    }

    return 0;
}