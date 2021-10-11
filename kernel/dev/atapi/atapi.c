#include "atapi.h"

#include <micro/module.h>
#include <micro/debug.h>

void atapi_init()
{
    printk("ATAPI driver loaded\n");    
}

void atapi_fini()
{
    printk("ATAPI driver finalizing\n");    
}

struct modmeta meta =
{
    .init = atapi_init,
    .fini = atapi_fini,
    .name = "ATAPI"
};
