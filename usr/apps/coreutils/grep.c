#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <regex.h>

void usage()
{
    printf("usage: grep <regex> <filename>\n");
}

void printsub(const char* start, size_t n)
{
    for (size_t i = 0; i < n; i++)
        fputc(start[i], stdout);
}

int main(int argc, char** argv)
{
    /*struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);

    char* buffer = malloc(1024);
    size_t size;
    if (argc == 3)
    {
        FILE* file = fopen(argv[2], "r");
        if (!file)
        {
            perror("grep");
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

    char* pattern = argv[1];*/
    FILE* file = fopen(argv[2], "r");

    // Compile and execute the regex
    regex_t regex;
    regcomp(&regex, argv[1], 0);

    size_t lineno = 1;
    char line[256];
    while (fgets(line, 256, file))
    {
        regmatch_t matches[32];
        const char* p = line;

        while (1)
        {
            int nom = regexec(&regex, p, 32, matches, 0);
            if (nom) break;
            
            for (size_t i = 0; i < 32; i++)
            {
                if (matches[i].rm_so == -1) break;

                printf("\033[93m%d\033[00m ", lineno);
                printsub(line, matches[i].rm_so);
                printf("\033[91m");
                printsub((char*)line + matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so); 
                printf("\033[00m");
                printf("%s", (char*)line + matches[i].rm_eo);
            }
            p += matches[0].rm_eo;
        }
        
        lineno++;
    }

    // TODO: regex instead of silly search
    /*char* saveptr;
    char* line = strtok_r(buffer, "\n", &saveptr);
    size_t lineno = 1;

    while (line)
    {
        for (size_t idx = 0; line[idx]; idx++)
        {
            if (!strncmp(pattern, line + idx, strlen(pattern)))
            {
                printf("\033[93m%d\033[00m ", lineno);
                for (size_t i = 0; i < idx; i++)
                    fputc(line[i], stdout);
                printf("\033[91m");
                printf("%s", pattern);
                printf("\033[00m");
                printf("%s\n", line + idx + strlen(pattern));
            }
        }

        line = strtok_r(NULL, "\n", &saveptr);
        lineno++;
    }*/

    return 0;
}
