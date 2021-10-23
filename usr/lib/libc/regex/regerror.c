#include <regex.h>
#include <assert.h>

size_t regerror(int errcode, const regex_t* preg, char* errbuf,
                size_t errbuf_size)
{
    assert(!"regerror() not implemented!\n");
    return 0;
}