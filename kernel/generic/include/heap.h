#pragma once

#include <types.h>

void* kmalloc(size_t n);
void kfree(void* ptr);
void heap_init();
