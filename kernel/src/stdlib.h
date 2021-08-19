/*
 *  Kernel minimal C library
 */

#pragma once

#include <types.h>

void memset(void* ptr, char c, size_t size);
void memcpy(void* dst, const void* src, size_t size);
