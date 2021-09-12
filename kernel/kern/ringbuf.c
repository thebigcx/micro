#include <micro/ringbuf.h>
#include <micro/heap.h>
#include <micro/stdlib.h>

struct ringbuf* ringbuf_create(size_t size)
{
    struct ringbuf* buf = kmalloc(sizeof(struct ringbuf));

    buf->read = 0;
    buf->write = 0;
    buf->size = size;
    buf->buffer = kmalloc(size);

    return buf;
}

void ringbuf_free(struct ringbuf* buf)
{
    kfree(buf->buffer);
    kfree(buf);
}

void ringbuf_read(struct ringbuf* buf, void* ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        memcpy(ptr++, buf->buffer + buf->read++, size);

        if (buf->read == buf->size)
            buf->read = 0;
    }
}

void ringbuf_write(struct ringbuf* buf, const void* ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        memcpy(ptr++, buf->buffer + buf->read++, size);

        if (buf->read == buf->size)
            buf->read = 0;
    }
}