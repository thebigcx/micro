#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>

#define ADD    0
#define REMOVE 1
#define SET    2

mode_t getperms(const char* file)
{
    struct stat buf;
    stat(file, &buf);
    return buf.st_mode & 0777;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("usage: chmod [reference][operator][mode] <file>\n");
        return -1;
    }

    mode_t mode   = 0;
    uint32_t mask = 0;
    int remove    = 0;

    switch (argv[1][0])
    {
        case 'u':
            mask = 0700;
            break;
        case 'g':
            mask = 0070;
            break;
        case 'o':
            mask = 0007;
            break;
        case 'a':
            mask = 0777;
            break;
        default:
            printf("chmod: invalid reference character '%c'\n", argv[1][0]);
            return -1;
    }

    switch (argv[1][1])
    {
        case '-':
            remove = 1;
        case '+': // Falls through
            mode = getperms(argv[2]);
            break;

        case '=':
            break;
        default:
            printf("chmod: invalid operator character '%c'\n", argv[1][1]);
            return -1;
    }
    
    char* modestr = &argv[1][2];

    while (*modestr != 0)
    {
        switch (*modestr)
        {
            case 'r':
                if (remove)
                    mode &= ~(0444 & mask);
                else
                    mode |= 0444 & mask;
                break;
            case 'w':
                if (remove)
                    mode &= ~(0222 & mask);
                else
                    mode |= 0222 & mask;
                break;
            case 'x':
                if (remove)
                    mode &= ~(0111 & mask);
                else
                    mode |= 0111 & mask;
                break;
            case 's':
            {
                uint32_t bit;
                if (mask == 0700) bit = S_ISUID;
                else if (mask == 0070) bit = S_ISGID;
                if (remove)
                    mode &= ~bit;
                else
                    mode |= bit;
                break;
            }
            case 't':
                if (remove)
                    mode &= ~S_ISVTX;
                else
                    mode |= S_ISVTX;
                break;
            default:
                printf("chmod: invalid mode character '%c'\n", *modestr);
                return -1;
        }
        modestr++;
    }

    if (chmod(argv[2], mode))
    {
        perror("chmod");
        return -1;
    }

    return 0;
}
