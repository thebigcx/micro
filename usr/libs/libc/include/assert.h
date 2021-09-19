#pragma once

void __assert_failed(const char* expr);

#define assert(expr) if (!(expr)) __assert_failed(#expr);
