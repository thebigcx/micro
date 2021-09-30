#include <libc/libc-internal.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

struct block
{
    struct block* next;
    struct block* prev;
    int used;
    size_t size;
};

static struct block* first;
static struct block* last;
static void* end;

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

// TODO: don't rely on 0x100000 being available
void* __libc_heap_expand(size_t size)
{
    return mmap((void*)0x100000, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
}

void __malloc_init()
{
    first = (struct block*)__libc_heap_expand(0x100000);
    first->next = NULL;
    first->prev = NULL;
    first->size = 0x100000;
    first->used = 0;
}

void* malloc(size_t n)
{
    // We return NULL on a 0-size malloc (free(NULL) is safe)
    if (n == 0)
        return NULL;

    if (n % 32 != 0) n += 32 - (n % 32);

    struct block* curr = first;
    while (curr != NULL)
    {
        if (!curr->used)
        {
            if (curr->size > n)
            {
                struct block* b = split(curr, n);
                b->used = 1;
                return b + 1;
            }
            else if (curr->size == n)
            {
                curr->used = 1;
                return curr + 1;
            }
        }

        curr = curr->next;
    }

    // TODO: try to expand the heap

    printf("malloc(): unable to allocate 0x%x bytes (out of memory)\n", n);
   	return NULL;
}

void free(void* ptr)
{
    if (!ptr)
        return;

    struct block* block = (struct block*)ptr - 1;
    block->used = 0;

    if (block->prev && !block->prev->used) block = combine(block->prev, block);
    if (block->next && !block->next->used) block = combine(block, block->next);
}

void* realloc(void* ptr, size_t size)
{
    if (!size)
    {
        free(ptr);
        return NULL;
    }

	void* new = malloc(size);
    if (!new)
    {
        printf("realloc(): unable to malloc new size\n");
        return NULL;
    }

    if (!ptr) return new;

    size_t osize = ((struct block*)ptr - 1)->size;
    memcpy(new, ptr, osize);
    free(ptr);

    return new;
}

void* calloc(size_t nitems, size_t size)
{
    void* p = malloc(nitems * size);
    if (!p) return NULL;
    
    memset(p, 0, nitems * size);
	return p;
}
