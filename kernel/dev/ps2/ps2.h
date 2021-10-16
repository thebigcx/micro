#pragma once

#include <micro/types.h>

uint8_t ps2_read();
void ps2_write(uint8_t cmd);
void ps2_write2(uint8_t cmd, uint8_t val);

void init_keyboard();
void init_mouse();
