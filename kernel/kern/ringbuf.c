#include <micro/ringbuf.h>
#include <micro/heap.h>
#include <micro/stdlib.h>

struct ringbuf* ringbuf_create(size_t size)
{
    struct ringbuf* buf = kmalloc(sizeof(struct ringbuf));

    buf->read   = 0;
    buf->write  = 0;
    buf->size   = size;
    buf->buffer = kmalloc(size);
    buf->full   = 0;

    return buf;
}

void ringbuf_free(struct ringbuf* buf)
{
    kfree(buf->buffer);
    kfree(buf);
}

// Increment write head
static void ringbuf_inc(struct ringbuf* buf)
{
    if (buf->full) // Start overwriting data
    {
        if (++(buf->read) == buf->size)
            buf->read = 0;
    }

    if (++(buf->write) == buf->size)
        buf->write = 0;

    buf->full = buf->write == buf->read;
}

// Increment read head
static void ringbuf_dec(struct ringbuf* buf)
{
    if (++(buf->read) == buf->size)
        buf->read = 0;

    buf->full = 0;
}

void ringbuf_read(struct ringbuf* buf, void* ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        *((uint8_t*)ptr++) = *((uint8_t*)(buf->buffer + buf->read));
        ringbuf_dec(buf);
    }
}

void ringbuf_write(struct ringbuf* buf, const void* ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        *((uint8_t*)(buf->buffer + buf->write)) = *((uint8_t*)ptr++);
        ringbuf_inc(buf);
    }
}

// difference between read and write head
size_t ringbuf_size(struct ringbuf* buf)
{
    if (!buf->full)
    {
        if (buf->write >= buf->read)
            return buf->write - buf->read;
        else
            return buf->size + buf->write - buf->read;
    }

    return buf->size;
}