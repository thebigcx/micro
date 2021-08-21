#include <lapic.h>
#include <cpu.h>
#include <debug/syslog.h>
#include <mmu.h>

// Local APIC registers
#define R_ID          0x020
#define R_VERS        0x030
#define R_TPR         0x080
#define R_APR         0x090
#define R_PPR         0x0a0
#define R_EOI         0x0b0
#define R_RRD         0x0c0
#define R_LDR         0x0d0
#define R_DFR         0x0e0
#define R_SIVR        0x0f0
#define R_ISR         0x100
#define R_TMR         0x180
#define R_IRR         0x200
#define R_ERR         0x280
#define R_ICRLO       0x300
#define R_ICRHI       0x310

#define R_LVT_TIME    0x320
#define R_LVT_THERM   0x330
#define R_LVT_PERF    0x340
#define R_LVT_LINT0   0x350
#define R_LVT_LINT1   0x360
#define R_LVT_ERR     0x370

#define R_TIME_INIT   0x380
#define R_TIME_CURR   0x390
#define R_TIME_DIVCFG 0x3e0

static volatile uintptr_t mmio_base;

static uintptr_t get_base()
{
    uintptr_t low, high;
    rdmsr(0x1b, low, high);
    return (high << 32) | low;
}

static void set_base(uintptr_t base)
{
    wrmsr(0x1b, base & 0xffffffff, base >> 32);
}

static void write(uint32_t off, uint32_t val)
{
    *((volatile uint32_t*)(mmio_base + off)) = val;
}

static uint32_t read(uint32_t off)
{
    return *((volatile uint32_t*)(mmio_base + off));
}

// Setup LAPIC for the BSP (map the MMIO)
void lapic_setup()
{
    mmio_base = mmu_map_mmio(get_base() & PAGE_FRAME);
    lapic_enable();
}

void lapic_enable()
{
    set_base(get_base() | (1 << 11));
    write(R_SIVR, read(R_SIVR) | 0x1ff);
}
