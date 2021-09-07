#pragma once

#include <micro/types.h>

struct fb
{
    void* addr;
    unsigned int width, height, bpp;
};

void fb_init(unsigned int width, unsigned int height, unsigned int depth);
void fb_set_addr(void* addr);

void fb_clear(uint32_t fg);
void fb_putch(char c, uint32_t fg, uint32_t bg);
void fb_print(const char* str, uint32_t fg, uint32_t bg);