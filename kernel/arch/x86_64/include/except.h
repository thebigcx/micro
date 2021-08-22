#pragma once

#include <types.h>

struct regs;

void except(uintptr_t n, struct regs* r, uint32_t e);
