#include <micro/ksym.h>
#include <micro/debug.h>
#include <micro/pci.h>
#include <arch/mmu.h>
#include <micro/heap.h>
#include <micro/vfs.h>
#include <micro/stdlib.h>
#include <micro/devfs.h>
#include <arch/descs.h>
#include <arch/cmos.h>

struct ksym
{
    const char* name;
    void*       value;
};

// TODO: define a macro called EXPORT_SYMBOL() to build
// this array automatically
static struct ksym ksyms[] =
{
    { "printk",                &printk                },
    { "pci_register_driver",   &pci_register_driver   },
    { "pci_enable_intrs",      &pci_enable_intrs      },
    { "pci_enable_bus_master", &pci_enable_bus_master },
    { "pci_enable_mmio",       &pci_enable_mmio       },
    { "pci_get_bar",           &pci_get_bar           },
    { "mmu_map_mmio",          &mmu_map_mmio          },
    { "mmu_alloc_phys",        &mmu_alloc_phys        },
    { "mmu_kalloc",            &mmu_kalloc            },
    { "mmu_kmap",              &mmu_kmap              },
    { "kmalloc",               &kmalloc               },
    { "kcalloc",               &kcalloc               },
    { "kfree",                 &kfree                 },
    { "vfs_open_new",              &vfs_open_new              },
    { "vfs_read_new",             &vfs_read_new             },
    { "vfs_write_new",             &vfs_write_new             },
    { "vfs_pread",             &vfs_pread             },
    { "vfs_pwrite",             &vfs_pwrite             },
    { "vfs_create_file",       &vfs_create_file       },
    { "vfs_resolve",           &vfs_resolve           },
    { "vfs_register_fs",       &vfs_register_fs       },
    { "vfs_access",            &vfs_access            },
    { "devfs_register_chrdev", &devfs_register_chrdev },
    { "devfs_register_blkdev", &devfs_register_blkdev },
    { "register_irq_handler",  &register_irq_handler  },
    { "time_getepoch",         &time_getepoch         }
};

uintptr_t ksym_lookup(const char* name)
{
    // Search the table for the symbol
    for (size_t i = 0; i < sizeof(ksyms) / sizeof(struct ksym); i++)
        if (!strcmp(ksyms[i].name, name)) return (uintptr_t)ksyms[i].value;

    printk("could not find symbol: %s\n", name);
    return 0;
}