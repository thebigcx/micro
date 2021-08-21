#include <ioapic.h>

#define R_REGSEL 0x00
#define R_IOWIN  0x10

// Redirection table entry
#define REDTBL(irq) (2 * irq + 0x10)

static volatile uintptr_t mmio_base;

uint32_t read32(uint32_t reg)
{
    *(volatile uint32_t*)(mmio_base + R_REGSEL) = reg;
    return *(volatile uint32_t*)(mmio_base + R_IOWIN);
}

void write32(uint32_t reg, uint32_t val)
{
    *(volatile uint32_t*)(mmio_base + R_REGSEL) = reg;
    *(volatile uint32_t*)(mmio_base + R_IOWIN)  = val;
}

void write64(uint32_t reg, uint64_t val)
{
    // Write two 32-bit values
    write32(reg, val & 0xffffffff);
    write32(reg + 1, val >> 32);
}

void ioapic_init(uintptr_t base)
{
    mmio_base = mmu_map_mmio(base);
}

void ioapic_redir(uint8_t irq, uint8_t vec, uint32_t deliv)
{
    write64(REDTBL(irq), (uint32_t)vec | deliv);
}
