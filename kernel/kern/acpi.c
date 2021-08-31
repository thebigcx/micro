#include <acpi.h>
#include <mmu.h>
#include <debug/syslog.h>
#include <stdlib.h>
#include <ioapic.h>
#include <list.h>

struct __attribute__((packed)) rsdp
{
    uint8_t sig[8];
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t rev;
    uint32_t rsdt_addr;
};

struct __attribute__((packed)) xsdp
{
    struct rsdp base;
    uint32_t len;
    uint64_t xsdt_addr;
    uint8_t ext_checksum;
    uint8_t res[3];
};

struct __attribute__((packed)) sdthdr
{
    uint8_t sig[4];
    uint32_t len;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t oem_tbl_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_rev;
};

struct __attribute__((packed)) rsdt
{
    struct sdthdr hdr;
    uint32_t sdts[];
};

struct __attribute__((packed)) xsdt 
{
    struct sdthdr hdr;
    uint64_t sdts[];
};

struct __attribute__((packed)) madt
{
    struct sdthdr hdr;
    uint32_t lapic_addr;
    uint32_t flags;
};

#define MADT_LAPIC          0
#define MADT_IOAPIC         1
#define MADT_IOAPIC_ISO     2
#define MADT_IOAPIC_NONMASK 3
#define MADT_LAPIC_NONMASK  4
#define MADT_LAPIC_ADDR     5
#define MADT_LAPIC_X2       9

struct __attribute__((packed)) madtent
{
    uint8_t type;
    uint8_t len;
};

struct __attribute__((packed)) apiciso
{
    struct madtent ent;
    uint8_t bus;
    uint8_t irq;  // IRQ interrupt vector
    uint32_t gsi; // Global interrupt
    uint16_t flags;
};

struct __attribute__((packed)) ioapic
{
    struct madtent ent;
    uint8_t id;
    uint8_t res;
    uint32_t addr;
    uint32_t gsib;
};

struct __attribute__((packed)) lapic
{
    struct madtent ent;
    uint8_t id;
    uint8_t apic_id;
    uint32_t flags;
};

// Enabled, online capable shorthand
#define LAPIC_USABLE (1 | (1 << 2))

static struct rsdt* rsdt;
static struct xsdt* xsdt;
static int rev;

static struct list lapics;

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

    lapics = list_create();
}

// TODO: put this somewhere else
void acpi_parse_madt()
{
    struct madt* madt = acpi_find("APIC");

    struct madtent* ent = (struct madtent*)((uintptr_t)madt + sizeof(struct madt));
    uintptr_t end = (uintptr_t)madt + madt->hdr.len;

    while ((uintptr_t)ent < end)
    {
        switch (ent->type)
        {
            case MADT_LAPIC:
            {
                struct lapic* lapic = (struct lapic*)ent;
                if (lapic->flags & LAPIC_USABLE && lapic->apic_id) // Make sure not BSP
                {
                    uint16_t* id = kmalloc(sizeof(uint16_t));
                    *id = lapic->apic_id;
                    list_push_back(&lapics, id);
                }
            }
            break;

            case MADT_IOAPIC:
            {
                struct ioapic* ioapic = (struct ioapic*)ent;
                if (!ioapic->gsib) ioapic_init(ioapic->addr);
            }
            break;

            case MADT_IOAPIC_ISO:
            {
                struct apiciso* iso = (struct apiciso*)ent;
                printk("found iso: %d to %d", iso->irq, iso->gsi);
            }
            break;

            case MADT_LAPIC_NONMASK:
            {

            }
            break;

            case MADT_LAPIC_ADDR:
            {

            }
            break;
        }

        ent = (struct madtent*)((uintptr_t)ent + ent->len);
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

struct list* acpi_get_lapics()
{
    return &lapics;
}
