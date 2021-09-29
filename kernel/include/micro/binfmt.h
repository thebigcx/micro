#pragma once

#include <micro/types.h>

struct task;
struct vm_map;

void setup_user_stack(struct task* task, const char* argv[],
                      const char* envp[]);
int elf_load(struct vm_map* task, void* data, const char* argv[],
             const char* envp[], uintptr_t* rip);
const char* elf_getinterp(void* data);