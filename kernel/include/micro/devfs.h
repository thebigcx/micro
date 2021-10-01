#pragma once

struct file;

void devfs_init();
void devfs_register(struct file* file, const char* name);