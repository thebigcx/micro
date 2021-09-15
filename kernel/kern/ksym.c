#include <micro/ksym.h>
#include <micro/debug.h>
#include <micro/pci.h>
#include <arch/mmu.h>

struct ksym
{
    const char* name;
    uintptr_t   value;
};

static struct ksym ksyms[] =
{
    { "printk",                printk                },
    { "pci_register_driver",   pci_register_driver   },
    { "pci_enable_intrs",      pci_enable_intrs      },
    { "pci_enable_bus_master", pci_enable_bus_master },
    { "pci_enable_mmio",       pci_enable_mmio       },
    { "pci_get_bar",           pci_get_bar           },
    { "mmu_map_mmio",          mmu_map_mmio          }
};

uintptr_t ksym_lookup(const char* name)
{
    // Search the table for the symbol
    for (size_t i = 0; i < sizeof(ksyms) / sizeof(struct ksym); i++)
        if (!strcmp(ksyms[i].name, name)) return ksyms[i].value;

    return 0;
}