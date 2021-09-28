#pragma once

#include <micro/types.h>

void* kmalloc(size_t n);
void* kcalloc(size_t n, ...);
void kfree(void* ptr);
void heap_init();
void heap_check();