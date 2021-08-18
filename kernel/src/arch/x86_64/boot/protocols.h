#pragma once

#include <types.h>

struct __attribute__((packed)) st2header
{
    uint64_t entry;
    uint64_t stack;
    uint64_t flags;
    uint64_t tags;
};

struct __attribute__((packed)) st2struct
{
    char bl_brand[64]; // Bootloader brand
    char bl_vers[64];  // Bootloader version

    uint64_t tags;
};
