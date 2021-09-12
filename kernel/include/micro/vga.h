#pragma once

#include <micro/types.h>

enum VGA_COLS
{
    VGA_BLACK,
    VGA_BLUE,
	VGA_GREEN,
	VGA_CYAN,
	VGA_RED,
	VGA_MAGENTA,
	VGA_BROWN,
	VGA_LGRAY,
	VGA_DGRAY,
	VGA_LBLUE,
	VGA_LGREEN,
	VGA_LCYAN,
	VGA_LRED,
	VGA_PINK,
	VGA_YELLOW,
	VGA_WHITE
};

void vga_set_addr(uintptr_t addr);
void vga_putc(char c);
void vga_set_color(int fg, int bg);