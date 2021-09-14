#include <micro/ksym.h>
#include <micro/debug.h>

struct ksym
{
    const char* name;
    uintptr_t   value;
};

static struct ksym ksyms[] =
{
    { "printk", printk }
};

uintptr_t ksym_lookup(const char* name)
{
    // Search the table for the symbol
    for (size_t i = 0; i < sizeof(ksyms) / sizeof(struct ksym); i++)
        if (!strcmp(ksyms[i].name, name)) return ksyms[i].value;

    return 0;
}