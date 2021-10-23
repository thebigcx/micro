#include <dlfcn.h>
#include <stddef.h>

// Stubs if the executable is not dynamic
void* dlopen(const char* file, int mode)
{
    return NULL;
}

void* dlsym(void* handle, const char* sym)
{
    return NULL;
}

int dlclose(void* handle)
{
    return 0;
}

char* dlerror()
{
    return "dl functions not available";
}