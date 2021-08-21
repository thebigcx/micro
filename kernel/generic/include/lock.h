#pragma once

typedef volatile int lock_t;

#define LOCK(n)\
    while (!__sync_bool_compare_and_swap(&n, 0, 1));\
    __sync_synchronize();

#define UNLOCK(n)\
    __sync_synchronize();\
    n = 0;
