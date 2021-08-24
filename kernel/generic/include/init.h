#pragma once

#include <types.h>

void* initrd_read(const char* file);
void initrd_init(uintptr_t start, uintptr_t end);
