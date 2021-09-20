#pragma once

#include <micro/types.h>

struct regs;

void timer_tick(struct regs* r);
void timer_wait(uint64_t ns);
void timer_init();

uint64_t timer_usec();