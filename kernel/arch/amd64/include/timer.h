#pragma once

#include <types.h>

struct regs;

void timer_tick(struct regs* r);
void timer_wait(uint64_t ns);
