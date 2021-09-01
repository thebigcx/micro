#pragma once

#include <types.h>

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