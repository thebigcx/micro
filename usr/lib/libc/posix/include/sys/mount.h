#pragma once

int mount(const char* src, const char* dst,
          const char* fstype, unsigned long flags,
          const void* data);
int umount(const char* target);