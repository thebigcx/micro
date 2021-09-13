#include <micro/acpi.h>
#include <micro/acpi_defs.h>
#include <arch/mmu.h>
#include <micro/debug.h>
#include <micro/stdlib.h>
#include <arch/ioapic.h>
#include <micro/list.h>

static struct rsdt* rsdt;
static struct xsdt* xsdt;
static int rev;

void acpi_init(uintptr_t rsdp)
{
    struct rsdp* ptr = (struct rsdp*)mmu_map_mmio(rsdp, 1);
    printk("rsdp: %x\n", ptr);
    rev = ptr->rev;

    if (rev == 0)
        rsdt = (struct rsdt*)mmu_map_mmio(ptr->rsdt_addr, 1);
    else
    {
        struct xsdp* extptr = (struct xsdp*)ptr;
        xsdt = (struct xsdt*)mmu_map_mmio(extptr->xsdt_addr, 1);
    }
}

void* acpi_find(const char* sig)
{
    unsigned int entries;

    if (rev == 0)
        entries = (rsdt->hdr.len - sizeof(rsdt->hdr)) / 4;
    else
        entries = (xsdt->hdr.len - sizeof(xsdt->hdr)) / 4;

    for (unsigned int i = 0; i < entries; i++)
    {
        struct sdthdr* h;
        if (rev == 0)
            h = (struct sdthdr*)mmu_map_mmio((uint64_t)rsdt->sdts[i], 1);
        else
            h = (struct sdthdr*)mmu_map_mmio(xsdt->sdts[i], 1);
        if (!strncmp((char*)h->sig, sig, 4)) return (void*)h;
    }

    return NULL;
}