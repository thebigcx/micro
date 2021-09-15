#include <micro/ksym.h>
#include <micro/debug.h>
#include <micro/pci.h>
#include <arch/mmu.h>
#include <micro/heap.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>

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
    { "mmu_map_mmio",          mmu_map_mmio          },
    { "mmu_alloc_phys",        mmu_alloc_phys        },
    { "mmu_kalloc",            mmu_kalloc            },
    { "mmu_kmap",              mmu_kmap              },
    { "kmalloc",               kmalloc               },
    { "vfs_addnode",           vfs_addnode           },
    { "vfs_read",              vfs_read              },
    { "kfree",                 kfree                 },
    { "vfs_write",             vfs_write             },
    { "vfs_create_file",       vfs_create_file       },
    { "vfs_resolve",           vfs_resolve           },
    { "vfs_register_fs",       vfs_register_fs       }, // TODO: inline these stdlib.c methods
    { "strcspn",       strcspn       },
    { "strlen",       strlen       },
    { "memcpy",       memcpy       },
    { "memset",       memset       },
    { "strncmp",       strncmp       },
    { "strcpy",       strcpy       },
    { "min",       min       },
};

uintptr_t ksym_lookup(const char* name)
{
    // Search the table for the symbol
    for (size_t i = 0; i < sizeof(ksyms) / sizeof(struct ksym); i++)
        if (!strcmp(ksyms[i].name, name)) return ksyms[i].value;

    printk("could not find symbol: %s\n", name);
    return 0;
}