#pragma once

#include <types.h>

struct list;

void acpi_init(uintptr_t rsdp);
void acpi_parse_madt();
void* acpi_find(const char* sig);
struct list* acpi_get_lapics();
