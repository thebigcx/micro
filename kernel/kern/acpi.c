#include <acpi.h>
#include <acpi_defs.h>
#include <mmu.h>
#include <debug/syslog.h>
#include <stdlib.h>
#include <ioapic.h>
#include <list.h>

static struct rsdt* rsdt;
static struct xsdt* xsdt;
static int rev;

void acpi_init(uintptr_t rsdp)
{
    struct rsdp* ptr = (struct rsdp*)rsdp;
    rev = ptr->rev;

    if (rev == 0)
        rsdt = (struct rsdt*)mmu_map_mmio(ptr->rsdt_addr);
    else
    {
        struct xsdp* extptr = (struct xsdp*)rsdp;
        xsdt = (struct xsdt*)mmu_map_mmio(extptr->xsdt_addr);
    }
}

void* acpi_find(const char* sig)
{
    int entries = (rsdt->hdr.len - sizeof(rsdt->hdr)) / 4;

    for (int i = 0; i < entries; i++)
    {
        struct sdthdr* h = (struct sdthdr*)((uint64_t)rsdt->sdts[i]);
        if (!strncmp((char*)h->sig, sig, 4)) return (void*)h;
    }

    return NULL;
}