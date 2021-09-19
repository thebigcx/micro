#pragma once

typedef unsigned long jmp_buf[8];

int setjmp(int environment);
void longjmp(jmp_buf environment, int value);