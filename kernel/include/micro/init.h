#pragma once

#include <micro/types.h>

struct genbootparams
{
    uintptr_t initrd_start;
    uintptr_t initrd_end;
};

void generic_init(struct genbootparams params);