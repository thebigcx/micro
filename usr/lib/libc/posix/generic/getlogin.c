#include <unistd.h>
#include <assert.h>

char* getlogin()
{
    assert(!"getlogin() is not implemented!\n");
    return NULL;
}