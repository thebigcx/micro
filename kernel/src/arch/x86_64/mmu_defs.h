#pragma once

#define PAGE4K 0x1000
#define PAGE2M 0x200000
#define PAGE1G 0x40000000

#define PAGE_PR     (1 << 0) // Present
#define PAGE_RW     (1 << 1) // Read/Write
#define PAGE_USR    (1 << 2) // User accessible
#define PAGE_WTHRU  (1 << 3) // Write-through cache
#define PAGE_DCACHE (1 << 4) // Disable cache
#define PAGE_ACC    (1 << 5) // Accessed (set by CPU)
#define PAGE_SZ     (1 << 6) // Size

typedef uint64_t pmlent_t;

struct __attribute__((aligned(PAGE4K)) pml
{
    pmlent_t entries[512];
};
