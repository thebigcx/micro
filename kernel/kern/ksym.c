#include <micro/ksym.h>
#include <micro/debug.h>

uintptr_t ksym_lookup(const char* name)
{
    // TODO: TEMP
    return (uintptr_t)printk;
}