#include <micro/debug.h>
#include <micro/types.h>

#if defined(DEBUG) && !defined(NOSANITIZE)

void __asan_load1_noabort(uintptr_t addr)
{

}

void __asan_load2_noabort(uintptr_t addr)
{
    
}

void __asan_load4_noabort(uintptr_t addr)
{
    
}

void __asan_load8_noabort(uintptr_t addr)
{
    
}

void __asan_load16_noabort(uintptr_t addr)
{
    
}

void __asan_loadN_noabort(uintptr_t addr, size_t size)
{
    
}

void __asan_store1_noabort(uintptr_t addr)
{

}

void __asan_store2_noabort(uintptr_t addr)
{
    
}

void __asan_store4_noabort(uintptr_t addr)
{
    
}

void __asan_store8_noabort(uintptr_t addr)
{
    
}

void __asan_store16_noabort(uintptr_t addr)
{
    
}

void __asan_storeN_noabort(uintptr_t addr, size_t size)
{
    
}

void __asan_handle_no_return()
{

}

#endif