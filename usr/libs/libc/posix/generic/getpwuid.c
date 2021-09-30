#include <pwd.h>
#include <assert.h>

struct passwd* getpwuid(uid_t uid)
{
    assert(!"getpwuid() not implemented!\n");
    return (void*)0;
}