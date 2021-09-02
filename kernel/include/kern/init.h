#pragma once

#include <types.h>

struct genbootparams
{
    uintptr_t initrd_start;
    uintptr_t initrd_end;
};

void initrd_init(uintptr_t start, uintptr_t end);

void generic_init(struct genbootparams params);