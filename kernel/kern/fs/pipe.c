#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/list.h>
#include <micro/ringbuf.h>
#include <micro/stdlib.h>

struct pipe
{
    struct ringbuf* buffer;

    struct file* reader;
    struct file* writer;
};

ssize_t pipe_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pipe* pipe = file->device;

    ssize_t bytes;
    while (!(bytes = ringbuf_size(pipe->buffer))) sched_yield();

    bytes = bytes < size ? bytes : size;

    ringbuf_read(pipe->buffer, buf, bytes);

    return bytes;
}

ssize_t pipe_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pipe* pipe = file->device;

    ssize_t bytes = size;
    //ssize_t bytes;
    //while (!(bytes = ringbuf_size(pipe->buffer))) sched_yield();

    ringbuf_write(pipe->buffer, buf, bytes);

    return bytes;
}

int pipe_create(struct file* files[2])
{
    struct pipe* pipe = kmalloc(sizeof(struct pipe));

    pipe->buffer = ringbuf_create(4096);

    files[0] = kmalloc(sizeof(struct file));
    files[1] = kmalloc(sizeof(struct file));

    memset(files[0], 0, sizeof(struct file));
    memset(files[1], 0, sizeof(struct file));

    files[0]->type = files[1]->type = S_IFIFO;
    files[0]->device = files[1]->device = pipe;

    files[0]->ops.read = pipe_read;
    files[1]->ops.write = pipe_write;

    files[0]->uid = files[1]->uid = task_curr()->euid;
    files[0]->gid = files[1]->gid = task_curr()->egid;

    files[0]->perms = files[1]->perms = 0777;
    
    pipe->reader = files[0];
    pipe->writer = files[1];

    return 0;
}

static int find_slots(int* slots, size_t nr)
{
    for (size_t i = 0; i < nr; i++)
    {
        int found = 0;
        for (size_t j = 0; j < FD_MAX; j++)
        {
            if (!task_curr()->fds[j] && (!i || j != (size_t)slots[i - 1]))
            {
                slots[i] = j;
                found = 1;
                break;
            }
        }

        if (!found)
            return -1;
    }

    return 0;
}

SYSCALL_DEFINE(pipe, int fds[2])
{
    struct file* files[2];
    int ret = pipe_create(files);

    if (ret) return ret;

    if (find_slots(fds, 2)) return -EMFILE;

    struct fd* fd1 = vfs_open(files[0], O_RDONLY, 0777);
    struct fd* fd2 = vfs_open(files[1], O_WRONLY, 0777);

    task_curr()->fds[fds[0]] = fd1;
    task_curr()->fds[fds[1]] = fd2;

    return 0;
}