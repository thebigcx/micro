#include "ansi.h"

#include <stdlib.h>

ansi_t ansi_init(struct ansicbs* cbs)
{
    struct ansistate* ansi = malloc(sizeof(struct ansistate));   
    ansi->cbs = *cbs;
    return (ansi_t)ansi;
}

int ansi_parse(ansi_t ansi, const char* seq)
{
    return 0;    
}
