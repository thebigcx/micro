#pragma once

#include <micro/types.h>

struct fb
{
    void* addr;
    unsigned int width, height, bpp;
};

void fb_init(struct fb* fb);

void fb_putch(char c, uint32_t fg, uint32_t bg);
void fb_print(const char* str, uint32_t fg, uint32_t bg);