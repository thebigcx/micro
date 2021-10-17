#include "ahci_defs.h"

#include <micro/debug.h>
#include <micro/module.h>
#include <micro/pci.h>
#include <micro/pci_ids.h>
#include <arch/mmu.h>
#include <micro/vfs.h>
#include <micro/heap.h>
#include <micro/stdlib.h>
#include <micro/devfs.h>
#include <micro/errno.h>
#include <micro/try.h>

struct ahci_port
{
    volatile struct hba_port* port;
    void*                     buffer;  // Mapped buffer
    uintptr_t                 pbuffer; // Physical buffer

    struct hba_cmd_hdr*       cmdlist;
    struct hba_cmd_tbl*       cmdtbls[32];
    struct hba_fis*           fis;
};

struct pci_driver pci_dri;

volatile struct hba_mem* abar;
volatile struct hba_mem* vabar;

void port_start_cmd(struct ahci_port* port)
{
    while (port->port->cmd_stat & HBA_PXCMD_CR);

    port->port->cmd_stat |= HBA_PXCMD_FRE;
    port->port->cmd_stat |= HBA_PXCMD_ST;
}

void port_stop_cmd(struct ahci_port* port)
{
    port->port->cmd_stat &= ~HBA_PXCMD_ST;
    port->port->cmd_stat &= ~HBA_PXCMD_FRE;

    while (1)
    {
        if (port->port->cmd_stat & HBA_PXCMD_FR) continue;
        if (port->port->cmd_stat & HBA_PXCMD_CR) continue;

        break;
    }
}

int port_find_cmd_slot(struct ahci_port* port)
{
    uint32_t slots = (port->port->sata_active | port->port->cmd_issue);

    for (int32_t i = 0; i < AHCI_CMD_SLOTS; i++)
    {
        if ((slots & i) == 0)
            return i;

        slots >>= 1;
    }

    printk("ahci: could not find free command list entry\n");
    return -1;
}

int port_access(struct ahci_port* port, uintptr_t lba, uint32_t cnt, int write)
{
    port->port->int_stat = 0xffffffff;
    
    int slot = port_find_cmd_slot(port);
    if (slot == -1) // No available command slots
        return -EIO;

    struct hba_cmd_hdr* cmdhdr = &port->cmdlist[slot];
    cmdhdr->cmd_fis_len = sizeof(struct fis_reg_h2d) / sizeof(uint32_t);
    cmdhdr->write       = write;
    cmdhdr->prdt_len    = (uint16_t)((cnt - 1) >> 4) + 1;

    struct hba_cmd_tbl* cmdtbl = port->cmdtbls[slot];
    memset(cmdtbl, 0, sizeof(struct hba_cmd_tbl));

    cmdtbl->prdt_entry[0].data_base_addr   = (uint32_t)port->pbuffer;
    cmdtbl->prdt_entry[0].data_base_addr_u = (uint32_t)(port->pbuffer >> 32);
    cmdtbl->prdt_entry[0].byte_cnt         = cnt * 512 - 1;
    cmdtbl->prdt_entry[0].int_on_cmpl      = 1;

    struct fis_reg_h2d* cmdfis = (struct fis_reg_h2d*)(&cmdtbl->cmd_fis);
    memset(cmdfis, 0, sizeof(struct fis_reg_h2d));

    cmdfis->fis_type = (uint8_t)FIS_TYPE_H2D;
    cmdfis->com_ctrl = 1; // Command
    cmdfis->command  = write ? ATA_CMD_WRITE_DMA_EX : ATA_CMD_READ_DMA_EX;

    cmdfis->lba0 = (uint8_t)(lba & 0xff);
    cmdfis->lba1 = (uint8_t)(lba >> 8  );
    cmdfis->lba2 = (uint8_t)(lba >> 16 );
    cmdfis->lba3 = (uint8_t)(lba >> 24 );
    cmdfis->lba4 = (uint8_t)(lba >> 32 );
    cmdfis->lba5 = (uint8_t)(lba >> 40 );

    cmdfis->device = 1 << 6; // LBA mode

    cmdfis->countl = cnt & 0xff;
    cmdfis->counth = (cnt >> 8) & 0xff;

    uint64_t spin = 0;

    while ((port->port->task_file_dat & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000) // Hung
        return -EIO;

    port->port->cmd_issue = 1 << slot;

    // Wait for completion
    while (port->port->cmd_issue & (1 << slot))
    {
        if (port->port->int_stat & HBA_PXIS_TFES) // Task file error
            return -EIO;
    }

    // Check again
    if (port->port->int_stat & HBA_PXIS_TFES) // Task file error
        return -EIO;

    return 0;
}

ssize_t port_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct ahci_port* port = file->inode->priv;

    uintptr_t lba   = off / 512;
    size_t    count = size / 512;

    ssize_t read = 0;
    while (count)
    {
        TRY(port_access(port, lba, count, 0));

        memcpy(buf, port->buffer, 512);
        buf = (void*)((uintptr_t)buf + 512);
        count--;
        lba++;
        read++;
    }
    
    return read * 512;
}

ssize_t port_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct ahci_port* port = file->inode->priv;

    uintptr_t lba   = off / 512;
    size_t    count = size / 512;

    size_t written = 0;
    while (count)
    {
        memcpy(port->buffer, buf, 512);

        TRY(port_access(port, lba, count, 1));

        buf = (void*)((uintptr_t)buf + 512);
        count--;
        lba++;
        written++;
    }

    return written * 512;
}

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

struct ahci_port* ahci_create_port(volatile struct hba_port* hba_port)
{
    struct ahci_port* port = kmalloc(sizeof(struct ahci_port));

    port->port = hba_port;

    port->pbuffer = mmu_alloc_phys();
    port->buffer  = (void*)mmu_kalloc(1);

    mmu_kmap((uintptr_t)port->buffer, port->pbuffer, PAGE_PR | PAGE_RW);

    port_stop_cmd(port);

    uintptr_t phys = mmu_alloc_phys();

    port->port->com_base_addr   = (uint32_t)(phys & 0xffffffff);
    port->port->com_base_addr_u = (uint32_t)(phys >> 32); // Upper 32 bits
    port->cmdlist = (struct hba_cmd_hdr*)mmu_map_mmio(phys, 1);

    memset(port->cmdlist, 0, PAGE_SIZE);

    phys = mmu_alloc_phys();

    port->port->fis_base   = (uint32_t)(phys & 0xffffffff);
    port->port->fis_base_u = (uint32_t)(phys >> 32); // Upper 32 bit
    port->fis = (struct hba_fis*)mmu_map_mmio(phys, 1);

    memset(port->fis, 0, PAGE_SIZE);

    port->fis->dsfis.fis_type = (uint8_t)FIS_TYPE_DMA_SETUP;
    port->fis->psfis.fis_type = (uint8_t)FIS_TYPE_PIO_SETUP;
    port->fis->rfis.fis_type  = (uint8_t)FIS_TYPE_H2D;
    port->fis->sdbfis[0]      = (uint8_t)FIS_TYPE_DEVBITS;

    for (uint32_t i = 0; i < 8; i++)
    {
        port->cmdlist[i].prdt_len = 1;
        
        phys = mmu_alloc_phys();

        port->cmdlist[i].cmd_tbl_base_addr   = (uint32_t)(phys & 0xffffffff);
        port->cmdlist[i].cmd_tbl_base_addr_u = (uint32_t)(phys >> 32);

        port->cmdtbls[i] = (struct hba_cmd_tbl*)mmu_map_mmio(phys, 1);
        memset(port->cmdtbls[i], 0, PAGE_SIZE);
    }

    port_start_cmd(port);

    return port;
}

// Initalize an AHCI controller
void ahci_init_ctrl()
{
    pci_enable_bus_master(&pci_dri);
    pci_enable_intrs(&pci_dri);
    pci_enable_mmio(&pci_dri);

    abar = (volatile struct hba_mem*)pci_get_bar(&pci_dri, 5);
    vabar = (volatile struct hba_mem*)mmu_map_mmio((uintptr_t)abar, 1);

    unsigned int ports_impl = vabar->ports_impl;

    for (unsigned int i = 0; i < 32; i++)
    {
        if ((ports_impl & (1 << i)))
        {
            int type = get_type(&vabar->ports[i]);

            if (type == AHCI_PORT_SATA || type == AHCI_PORT_SATAPI)
            {
                struct file_ops ops = { .read = port_read, .write = port_write };
                devfs_register_blkdev(&ops, "sda", 0660, ahci_create_port(&vabar->ports[i]));
            }
        }
    }
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