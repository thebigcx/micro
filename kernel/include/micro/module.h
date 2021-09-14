#pragma once

#include <micro/types.h>

struct modmeta
{
    void        (*init)();
    void        (*fini)();
    const char*   name;
};

struct module
{
    uintptr_t       addr;
    size_t          size;
    struct modmeta* meta;
};

int module_load(void* data, size_t len);
int module_free(const char* name);

void modules_init();