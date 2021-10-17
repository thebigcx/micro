#pragma once

#include <micro/types.h>

struct elfinf
{
    uintptr_t entry, brk;
};

struct task;
struct vm_map;

void setup_user_stack(struct task* task, char* const argv[],
                      char* const envp[]);
int elf_valid(void* data);
int elf_load(struct vm_map* map, void* data, char* const argv[],
             char* const envp[], struct elfinf* inf);
const char* elf_getinterp(void* data);
