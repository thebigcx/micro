#pragma once

#include <micro/types.h>

struct ringbuf
{
    uintptr_t read;
    uintptr_t write;
    size_t    size;
    void*     buffer;
};

struct ringbuf* ringbuf_create(size_t size);
void ringbuf_free(struct ringbuf* buf);

void ringbuf_read(struct ringbuf* buf, void* ptr, size_t size);
void ringbuf_write(struct ringbuf* buf, const void* ptr, size_t size);