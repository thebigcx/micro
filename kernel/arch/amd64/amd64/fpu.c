#include <arch/fpu.h>
#include <arch/cpu_func.h>

extern void enable_sse();
extern void enable_avx();

void fpu_init()
{
    enable_sse();
    //enable_avx();
}