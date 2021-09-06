#pragma once

#include <micro/types.h>

struct file;

void ps2_init();
ssize_t kb_read(struct file* file, void* buf, off_t off, size_t size);