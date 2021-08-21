#pragma once

#include <types.h>

void acpi_init(uintptr_t rsdp);
void acpi_parse_madt();
void* acpi_find(const char* sig);
