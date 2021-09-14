#include <micro/debug.h>
#include <micro/module.h>

void mod_init()
{
    printk("Module!\n");
}

void mod_fini()
{
    printk("Finalizing\n");
}

struct modmeta meta =
{
    .init = mod_init,
    .fini = mod_fini,
    .name = "test"
};