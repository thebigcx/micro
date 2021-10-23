#include <bits/confname.h>
#include <errno.h>
#include <limits.h>

long pathconf(const char* path, int name)
{
    switch (name)
    {
        case _PC_PATH_MAX:
            return PATH_MAX;
    }

    return -EINVAL;
}