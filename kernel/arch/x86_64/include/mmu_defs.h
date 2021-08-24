#pragma once

#include <types.h>

#define PAGE4K 0x1000
#define PAGE2M 0x200000
#define PAGE1G 0x40000000

#define PAGE_FRAME   0xffffFFFFffffF000

#define PAGE_PR      (1 << 0) // Present
#define PAGE_RW      (1 << 1) // Read/Write
#define PAGE_USR     (1 << 2) // User accessible
#define PAGE_WTHRU   (1 << 3) // Write-through cache
#define PAGE_NOCACHE (1 << 4) // Disable cache
#define PAGE_ACC     (1 << 5) // Accessed (set by CPU)
#define PAGE_SZ      (1 << 6) // Size

#define PD_2M        (1 << 7) // 2 megabyte pages

#define _pagealign __attribute__((aligned(PAGE4K)))

typedef uint64_t page_t;
typedef page_t pml_t[512];

// Process page directory
struct pagedir
{
    pml_t* pml4;
    uintptr_t pml4_phys;

    pml_t* pdpt;
    uintptr_t pdpt_phys;

    page_t** pds;
    uintptr_t pds_phys[512];

    page_t*** tbls;
};
