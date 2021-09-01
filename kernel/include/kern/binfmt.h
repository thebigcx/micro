#pragma once

#include <types.h>

struct task;

uintptr_t elf_load(struct task* task, void* data);