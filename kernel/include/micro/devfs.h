#pragma once

#include <micro/types.h>

struct file;
struct file_ops;

void devfs_init();

void devfs_register_chrdev(struct file_ops* ops, const char* name, mode_t mode, void* priv);
void devfs_register_blkdev(struct file_ops* ops, const char* name, mode_t mode, void* priv);