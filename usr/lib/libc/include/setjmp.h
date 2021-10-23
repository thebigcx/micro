#pragma once

typedef unsigned long jmp_buf[8];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int value);
int _setjmp(jmp_buf env);
void _longjmp(jmp_buf env, int value);