#pragma once

struct modmeta
{
    void (*init)();
    void (*fini)();
};

void module_load(const char* path);