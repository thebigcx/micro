#include <micro/pci.h>
#include <arch/pio.h>

#define PCI_CFG_ADDR 0xcf8
#define PCI_CFG_DATA 0xcfc

#define PCI_BUS_CNT     256
#define PCI_DEV_CNT     32
#define PCI_FUNC_CNT    8

#define PCI_CMD_IO_SPACE     (1 << 0 )
#define PCI_CMD_MEM_SPACE    (1 << 1 )
#define PCI_CMD_BUS_MASTER   (1 << 2 )
#define PCI_CMD_INTS_DISABLE (1 << 10)

#define PCI_VENDOR_ID           0x00
#define PCI_DEVICE_ID           0x02
#define PCI_COMMAND             0x04
#define PCI_STATUS              0x06
#define PCI_REVISION_ID         0x08
#define PCI_PROGIF              0x09
#define PCI_SUBCLASS            0x0a
#define PCI_CLASS_CODE          0x0b
#define PCI_CACHE_SIZE          0x0c
#define PCI_LATENCY_TIMER       0x0d
#define PCI_HDR_TYPE            0x0e
#define PCI_BIST                0x0f
#define PCI_BAR0                0x10
#define PCI_BAR1                0x14
#define PCI_BAR2                0x18
#define PCI_BAR3                0x1c
#define PCI_BAR4                0x20
#define PCI_BAR5                0x24
#define PCI_CARDBUS_CIS_PTR     0x28
#define PCI_SUBSYS_VENDOR_ID    0x2c
#define PCI_SUBSYS_ID           0x2e
#define PCI_EXPAN_ROM_BASE_ADDR 0x30
#define PCI_CAPAB_PTR           0x34
#define PCI_INTERRUPT_LINE      0x3c
#define PCI_INTERRUPT_PIN       0x3d
#define PCI_MIN_GRANT           0x3e
#define PCI_MAX_LATENCY         0x3f

static uint32_t cfg_addr(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
    return (uint32_t)( ((uint32_t)bus  << 16 ) |
                       ((uint32_t)dev << 11 ) |
                       ((uint32_t)func << 8  ) |
                       (off & 0xfc)            |
                       ((uint32_t)0x80000000));
}

static uint32_t cfg_readl(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
    outl(PCI_CFG_ADDR, cfg_addr(bus, dev, func, off));
    return inl(PCI_CFG_DATA);
}

static uint16_t cfg_readw(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
    outl(PCI_CFG_ADDR, cfg_addr(bus, dev, func, off));
    return (uint16_t)((inl(PCI_CFG_DATA) >> ((off & 2) * 8)) & 0xffff);
}

static uint8_t cfg_readb(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off)
{
    outl(PCI_CFG_ADDR, cfg_addr(bus, dev, func, off));
    return (uint8_t)((inl(PCI_CFG_DATA) >> ((off & 3) * 8)) & 0xff);
}

static void cfg_writel(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off, uint32_t data)
{
    outl(PCI_CFG_ADDR, cfg_addr(bus, dev, func, off));
    outl(PCI_CFG_DATA, data);
}

static void cfg_writew(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off, uint16_t data)
{
    outl(PCI_CFG_ADDR, cfg_addr(bus, dev, func, off));
    outl(PCI_CFG_DATA, (inl(PCI_CFG_DATA) & (~(0xffff << ((off & 2) * 8)))) | ((uint32_t)data << ((off & 2) * 8)));
}

static void cfg_writeb(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off, uint8_t data)
{
    outl(PCI_CFG_ADDR, cfg_addr(bus, dev, func, off));
    outb(PCI_CFG_DATA, (inl(PCI_CFG_DATA) & (~(0xff << ((off & 3) * 8)))) | ((uint32_t)data << ((off & 3) * 8)));
}

// Check if a device exists
static int check_dev(uint8_t bus, uint8_t dev, uint8_t func)
{
    return (cfg_readw(bus, dev, func, PCI_VENDOR_ID) != 0xffff);
}

static void try_register(uint8_t bus, uint8_t dev, uint8_t func, struct pci_driver* dri)
{
    uint8_t class    = cfg_readb(bus, dev, func, PCI_CLASS_CODE);
    uint8_t subclass = cfg_readb(bus, dev, func, PCI_SUBCLASS);
    uint8_t progif   = cfg_readb(bus, dev, func, PCI_PROGIF);

    if (dri->id.class    == class
     && dri->id.subclass == subclass
     && dri->id.progif   == progif)
    {
        dri->bus  = bus;
        dri->dev  = dev;
        dri->func = func;
    }
}

void pci_enable_bus_master(struct pci_driver* dri)
{
    uint16_t data = cfg_readw(dri->bus, dri->dev, dri->func, PCI_COMMAND) | PCI_CMD_BUS_MASTER;
    cfg_writew(dri->bus, dri->dev, dri->func, PCI_COMMAND, data);
}

void pci_enable_intrs(struct pci_driver* dri)
{
    uint16_t data = cfg_readw(dri->bus, dri->dev, dri->func, PCI_COMMAND) & (~PCI_CMD_INTS_DISABLE);
    cfg_writew(dri->bus, dri->dev, dri->func, PCI_COMMAND, data);
}

void pci_enable_mmio(struct pci_driver* dri)
{
    uint16_t data = cfg_readw(dri->bus, dri->dev, dri->func, PCI_COMMAND) | PCI_CMD_MEM_SPACE;
    cfg_writew(dri->bus, dri->dev, dri->func, PCI_COMMAND, data);
}

void pci_enable_io(struct pci_driver* dri)
{
    uint16_t data = cfg_readw(dri->bus, dri->dev, dri->func, PCI_COMMAND) | PCI_CMD_IO_SPACE;
    cfg_writew(dri->bus, dri->dev, dri->func, PCI_COMMAND, data);
}

uintptr_t pci_get_bar(struct pci_driver* dri, uint8_t idx)
{
    uintptr_t bar = cfg_readl(dri->bus, dri->dev, dri->func, PCI_BAR0 + (idx * sizeof(uint32_t)));
    return (bar & 0x1) ? (bar & 0xFFFFFFFFFFFFFFFC) : (bar & 0xFFFFFFFFFFFFFFF0);
}

void pci_register_driver(struct pci_driver* dri)
{
    uint8_t func = 0;

    for (uint16_t bus = 0; bus < PCI_BUS_CNT; bus++)
    for (uint16_t dev = 0; dev < PCI_DEV_CNT; dev++)
    {
        if (check_dev(bus, dev, 0))
        {
            try_register(bus, dev, 0, dri);
            if ((cfg_readb(bus, dev, func, PCI_HDR_TYPE) & 0x80) != 0)
            {
                for (func = 1; func < PCI_FUNC_CNT; func++)
                {
                    if (check_dev(bus, dev, func))
                        try_register(bus, dev, func, dri);
                }
            }
        }
    }
}