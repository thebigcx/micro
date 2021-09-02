#pragma once

#include <micro/types.h>

void ioapic_init();
void ioapic_redir(uint8_t irq, uint8_t vec, uint32_t deliv);
