#include <arch/ioapic.h>
#include <arch/lapic.h>
#include <arch/mmu.h>
#include <arch/pic.h>
#include <micro/acpi.h>
#include <micro/acpi_defs.h>

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

void ioapic_init()
{
    unsigned int iso_cnt = 0;
    struct apiciso* isos[64];

    struct madt* madt = (struct madt*)acpi_find("APIC");

    struct madtent* ent = (struct madtent*)(madt + 1);
    uintptr_t end = (uintptr_t)madt + madt->hdr.len;

    while ((uintptr_t)ent < end)
    {
        switch (ent->type)
        {
            case MADT_IOAPIC:
            {
                struct ioapic* ioapic = (struct ioapic*)ent;
                if (!ioapic->gsib)
                {
                    mmio_base = mmu_map_mmio(ioapic->addr, 1);
                }
            }
            break;

            case MADT_IOAPIC_ISO:
            {
                isos[iso_cnt++] = (struct apiciso*)ent;
            }
            break;
        }

        ent = (struct madtent*)((uintptr_t)ent + ent->len);
    }

    pic_disable();

    for (unsigned int i = 0; i < iso_cnt; i++)
        ioapic_redir(isos[i]->gsi, isos[i]->irq + 32, DELIV_LOWEST);
}

void ioapic_redir(uint8_t irq, uint8_t vec, uint32_t deliv)
{
    write64(REDTBL(irq), (uint32_t)vec | deliv);
}