#pragma once

#include <sys/types.h>

int setgroups(size_t size, const gid_t* list);