#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

void __assert_failed(const char* expr)
{
    printf("Assertion failed: %s\n", expr);
    abort();
}