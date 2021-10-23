#pragma once

#define RTLD_LAZY   0
#define RTLD_NOW    1
#define RTLD_GLOBAL 2
#define RTLD_LOCAL  3

void* dlopen(const char* file, int mode);
void* dlsym(void* handle, const char* sym);
int   dlclose(void* handle);
char* dlerror();