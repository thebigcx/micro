#include <regex.h>
#include <assert.h>

int regexec(const regex_t* preg, const char* string, size_t nmatch,
            regmatch_t* pmatch, int eflags)
{
    assert(!"regexec() not implemented!\n");
    return -1;
}