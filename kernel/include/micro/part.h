#pragma once

#include <micro/types.h>
#include <micro/vfs.h>

// Partition device
struct partdev
{
    uint64_t start, end;
    struct file dev;
};

void register_part(uint64_t start, uint64_t end, struct file* dev, const char* name);
