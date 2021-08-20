#include <acpi.h>
#include <mmu.h>
#include <debug/syslog.h>
#include <stdlib.h>

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

struct __attribute__((packed)) madtent
{
    uint8_t type;
    uint8_t len;
};

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

    struct madt* madt = acpi_find("APIC");
    dbglnf("%x", madt);
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
