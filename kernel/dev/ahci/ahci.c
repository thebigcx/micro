#include "ahci_defs.h"

#include <micro/debug.h>
#include <micro/module.h>
#include <micro/pci.h>
#include <micro/pci_ids.h>
#include <arch/mmu.h>

struct pci_driver pci_dri;

volatile struct hba_mem* abar;
volatile struct hba_mem* vabar;

static int get_type(volatile struct hba_port* port)
{
    uint32_t sata_stat = port->sata_stat;

    uint8_t ipm = (sata_stat >> 8) & 0xf;
    uint8_t det = sata_stat & 0xf;

    if (det != HBA_PORT_DET_PRESENT)
        return AHCI_PORT_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_PORT_NULL;

    switch (port->sig)
    {
        case SATA_SIG_ATA:
            return AHCI_PORT_SATA;
        case SATA_SIG_ATAPI:
            return AHCI_PORT_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_PORT_SEMB;
        case SATA_SIG_PM:
            return AHCI_PORT_PM;
        default:
            return AHCI_PORT_NULL;
    }
}

// Initalize an AHCI controller
void ahci_init_ctrl()
{
    printk("ahci\n");

    pci_enable_bus_master(&pci_dri);
    pci_enable_intrs(&pci_dri);
    pci_enable_mmio(&pci_dri);

    /*abar = (volatile struct hba_mem*)pci_get_bar(&pci_dri, 5);
    vabar = (volatile struct hba_mem*)mmu_map_mmio((uintptr_t)abar, 1);

    unsigned int ports_impl = vabar->ports_impl;

    for (unsigned int i = 0; i < 32; i++)
    {
        if ((ports_impl & (1 << i)))
        {
            int type = get_type(&vabar->ports[i]);

            if (type == AHCI_PORT_SATA || type == AHCI_PORT_SATAPI)
            {
                printk("ahci: found disk\n");



                //m_ports.push_back(new Port(&m_vabar->ports[i], type, m_ports.size()));
                //FS::mount(m_ports.back(), "/dev/sda"); 
            }
        }
    }*/
}

void ahci_init()
{
    printk("loaded AHCI driver\n");

    pci_dri.id = (struct pci_id)
    {
        .class    = PCI_CLASS_STORAGE,
        .subclass = PCI_SC_SATA,
        .progif   = PCI_PI_AHCI
    };

    pci_register_driver(&pci_dri);
    ahci_init_ctrl();
}

void ahci_fini()
{
    printk("finalizing AHCI driver\n");
}

struct modmeta meta =
{
    .init = ahci_init,
    .fini = ahci_fini,
    .name = "ahci"
};