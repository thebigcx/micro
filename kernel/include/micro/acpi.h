#pragma once

#include <micro/types.h>

void acpi_init(uintptr_t rsdp);
void* acpi_find(const char* sig);