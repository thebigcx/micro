#pragma once

#include <micro/types.h>

struct task;
struct vm_map;

void setup_user_stack(struct task* task, char* const argv[],
                      char* const envp[]);
int elf_load(struct vm_map* task, void* data, char* const argv[],
             char* const envp[], uintptr_t* rip, uintptr_t* brk);
const char* elf_getinterp(void* data);
