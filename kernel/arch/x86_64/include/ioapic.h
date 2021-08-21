#pragma once

#include <types.h>

void ioapic_init(uintptr_t base);
void ioapic_map_irq(uint8_t irq);
