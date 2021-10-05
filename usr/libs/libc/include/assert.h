#pragma once

void __assert_failed(const char* expr);

#define assert(expr) __extension__ ({\
        if (!(expr)) __assert_failed(#expr);\
    })