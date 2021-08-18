#include <boot/protocols.h>
#include <types.h>

static uint8_t stack[4096];

__attribute__((section(".stivale2hdr"), used))
static struct st2header st2hdr =
{
    .entry = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = 0,
    .tags = 0
};

void kmain_st2(struct st2struct* st2struct)
{
    (void)st2struct;
    for (;;); 
}
