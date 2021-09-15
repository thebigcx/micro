#pragma once

#include <micro/types.h>

struct pci_id
{
    uint8_t class;
    uint8_t subclass;
    uint8_t progif;
};

struct pci_driver
{
    struct pci_id id;
    uint8_t       bus;
    uint8_t       dev;
    uint8_t       func;
};

void pci_register_driver(struct pci_driver* dri);
void pci_enable_bus_master(struct pci_driver* dri);

void pci_enable_intrs(struct pci_driver* dri);
void pci_enable_mmio(struct pci_driver* dri);
void pci_enable_io(struct pci_driver* dri);

uintptr_t pci_get_bar(struct pci_driver* dri, uint8_t idx);