#pragma once

enum
{
    _PC_PATH_MAX
};

long fpathconf(int fd, int name);
long pathconf(const char* path, int name);