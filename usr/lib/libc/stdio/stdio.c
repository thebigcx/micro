#include <stdio.h>

FILE __stdin =
{
    .fd  = 0,
    .err = 0,
    .eof = 0
};

FILE __stdout =
{
    .fd  = 1,
    .err = 0,
    .eof = 0
};

FILE __stderr =
{
    .fd  = 2,
    .err = 0,
    .eof = 0
};

FILE* stdin  = &__stdin;
FILE* stdout = &__stdout;
FILE* stderr = &__stderr;