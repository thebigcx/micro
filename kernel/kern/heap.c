#include <micro/heap.h>
#include <arch/mmu.h>
#include <micro/debug.h>
#include <micro/lock.h>
#include <micro/stdlib.h>

struct block
{
    struct block* next;
    struct block* prev;
    int used;
    size_t size;
};

static struct block* first;
static struct block* last;
static lock_t lock = 0;

// Merge b2 into b1 (both must be free)
// b1 and b2 are adjacent and in ascending order
static struct block* combine(struct block* b1, struct block* b2)
{
    // Add the size and adjust 'next' to jump over b2
    b1->size += b2->size + sizeof(struct block);
    b1->next = b2->next;

    // Try to adjust b1's next prev, if not than adjust tail pointer
    if (b1->next)
        b1->next->prev = b1;
    else
        last = b1;

    return b1;
}

static struct block* split(struct block* b, size_t n)
{
    struct block* new = (struct block*)((uintptr_t)b + sizeof(struct block) + n);
    memset(new, 0, sizeof(struct block));
   
    // Set the data
    new->used = 0;
    new->size = b->size - n - sizeof(struct block);
    
    b->size = n;
    
    // Adjust the pointers
    new->next = b->next;
    b->next = new;
    new->prev = b;
    
    // Try to adjust the 'next' field, if not than adjust tail pointer
    if (new->next)
        new->next->prev = new;
    else
        last = new;

    return b;
}

void* kmalloc(size_t n)
{
    if (n == 0) return NULL;

    if (n % 32 != 0) n += 32 - (n % 32);

    LOCK(lock);

    struct block* curr = first;
    while (curr != NULL)
    {
        if (!curr->used)
        {
            if (curr->size > n)
            {
                heap_check();
                struct block* b = split(curr, n);
                b->used = 1;
                UNLOCK(lock);
                heap_check();
//#ifdef DEBUG
                memset((void*)(b + 1), 0xcb, b->size);
//#endif
                heap_check();
                return b + 1;
            }
            else if (curr->size == n)
            {
                curr->used = 1;
                UNLOCK(lock);
//#ifdef DEBUG
                memset((void*)(curr + 1), 0xcb, curr->size);
//#endif
                heap_check();
                return curr + 1;
            }
        }

        curr = curr->next;
    }

    printk("unable to kmalloc: %x bytes, last block size: %x\n", n, last->size);
    UNLOCK(lock);
    return NULL;
}

void kfree(void* ptr)
{
    LOCK(lock);

    heap_check();

    struct block* block = (struct block*)ptr - 1;
    block->used = 0;
#ifdef DEBUG
    memset(block + 1, 0xcb, block->size);
#endif

    if (block->prev && !block->prev->used) block = combine(block->prev, block);
    if (block->next && !block->next->used) block = combine(block, block->next);

    heap_check();

    UNLOCK(lock);
}

void heap_init()
{
    uintptr_t start = mmu_kalloc(10000);
    for (unsigned int i = 0; i < 10000; i++)
        mmu_kmap(start + i * PAGE4K, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    first = last = (struct block*)start;
    first->used = 0;
    first->size = 10000 * PAGE4K;
    first->next = NULL;
    first->prev = NULL;
}

// Scan the heap to find corrupted blocks - crazy large sizes, invalid prev and next pointers
// 'used' flag not a 0 or a 1
// This is *extremely* helpful for finding heap buffer overflows, though it needs to be run
// effectively as often as possibly or between critical code
void heap_check()
{
    //return;
#ifdef DEBUG
    struct block* curr = first;
    while (curr != NULL)
    {
        if (curr != last && (uintptr_t)curr->next < 0xffffffffc0000000)
            panic("Heap Check failed\n");

        if (curr != first && (uintptr_t)curr->prev < 0xffffffffc0000000)
            panic("Heap Check failed\n");

        if ((curr->used != 0 && curr->used != 1) || curr->size > 0x10000000)
        {
            panic("Heap Check failed\n");
        }

        curr = curr->next;
    }
#endif
}