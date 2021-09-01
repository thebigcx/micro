#pragma once

#include <types.h>

void ioapic_init(uintptr_t base);
void ioapic_redir(uint8_t irq, uint8_t vec, uint32_t deliv);
