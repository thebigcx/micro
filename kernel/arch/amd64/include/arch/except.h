#pragma once

#include <micro/types.h>

struct regs;

void backtrace(uintptr_t rip, uintptr_t rbp, uint32_t maxframes);
void except(uintptr_t n, struct regs* r, uint32_t e);