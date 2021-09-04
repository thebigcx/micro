#include <micro/debug.h>
#include <micro/module.h>

void mod_init()
{
    printk("Module!\n");
}

void mod_fini()
{

}

struct modmeta meta =
{
    .init = mod_init,
    .fini = mod_fini
};