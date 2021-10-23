#pragma once

#include <stdint.h>

void __libc_init();
void __malloc_init();

uint32_t __libc_fopen_flags(const char* str);