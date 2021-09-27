#include <locale.h>

extern char* __en_US_langinfo[];

char* setlocale(int category, const char* locale)
{
    return (char*)locale;
}