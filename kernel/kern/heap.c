#include <heap.h>
#include <mmu.h>
#include <debug/syslog.h>
#include <lock.h>

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
static void combine(struct block* b1, struct block* b2)
{
    // Add the size and adjust 'next' to jump over b2
    b1->size += b2->size + sizeof(struct block);
    b1->next = b2->next;

    // Try to adjust b1's next prev, if not than adjust tail pointer
    if (b1->next)
        b1->next->prev = b1;
    else
        last = b1;
}

static struct block* split(struct block* b, size_t n)
{
    struct block* new = (struct block*)((uintptr_t)b + sizeof(struct block) + n);
   
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

    if (n % 16 != 0) n += 16 - (n % 16);

    LOCK(lock);

    struct block* curr = first;
    while (curr != NULL)
    {
        if (!curr->used)
        {
            if (curr->size > n)
            {
                struct block* b = split(curr, n);
                b->used = 1;
                UNLOCK(lock);
                return b + 1;
            }
            else if (curr->size == n)
            {
                curr->used = 1;
                UNLOCK(lock);
                return curr + 1;
            }
        }

        curr = curr->next;
    }

    printk("unable to kmalloc: %x bytes, last block size: %x", n, last->size);
    UNLOCK(lock);
    return NULL;
}

void kfree(void* ptr)
{
    LOCK(lock);

    struct block* block = (struct block*)ptr - 1;
    block->used = 0;

    if (block->prev && !block->prev->used) combine(block->prev, block);
    if (block->next && !block->next->used) combine(block, block->next);

    UNLOCK(lock);
}

void heap_init()
{
    void* start = mmu_kalloc(10000);
    for (int i = 0; i < 10000; i++)
        mmu_kmap(start + i * PAGE4K, mmu_alloc_phys(), PAGE_PR | PAGE_RW);

    first = last = (struct block*)start;
    first->used = 0;
    first->size = 10000 * PAGE4K;
    first->next = NULL;
    first->prev = NULL;
}
