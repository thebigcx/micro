#include <micro/sys.h>
#include <micro/vfs.h>
#include <micro/fcntl.h>
#include <micro/list.h>
#include <micro/ringbuf.h>
#include <micro/stdlib.h>
#include <micro/sched.h>

struct pipe
{
    struct ringbuf* buffer;

    struct inode* reader;
    struct inode* writer;

    int closed;
};

ssize_t pipe_read(struct file* file, void* buf, off_t off, size_t size)
{
    struct pipe* pipe = file->inode->priv;

    ssize_t bytes;
    while (!(bytes = ringbuf_size(pipe->buffer)))
    {
        if (pipe->closed)
        {
            task_send(task_curr(), SIGPIPE);
            return -EINTR;
        }

        sched_yield();
    }

    bytes = bytes < (ssize_t)size ? bytes : (ssize_t)size;

    ringbuf_read(pipe->buffer, buf, bytes);

    return bytes;
}

ssize_t pipe_write(struct file* file, const void* buf, off_t off, size_t size)
{
    struct pipe* pipe = file->inode->priv;

    // TODO: use thread_block()
    while (pipe->buffer->full)
    {
        if (pipe->closed)
        {
            task_send(task_curr(), SIGPIPE);
            return -EINTR;
        }

        sched_yield();
    }

    ssize_t empty = pipe->buffer->size - ringbuf_size(pipe->buffer);

    ssize_t bytes = (ssize_t)size < empty ? (ssize_t)size : empty;
    ringbuf_write(pipe->buffer, buf, bytes);

    return bytes;
}

// TODO: release resources and stuff
int pipe_close(struct file* file)
{
    struct pipe* pipe = file->inode->priv;
    pipe->closed = 1;
    return 0;
}

int pipe_create(struct inode* files[2])
{
    struct pipe* pipe = kcalloc(sizeof(struct pipe));

    pipe->buffer = ringbuf_create(4096);

    files[0] = kcalloc(sizeof(struct inode));
    files[1] = kcalloc(sizeof(struct inode));

    files[0]->mode = files[1]->mode = S_IFIFO | 0777;
    files[0]->priv = files[1]->priv = pipe;

    files[0]->fops.read = pipe_read;
    files[1]->fops.write = pipe_write;
    files[0]->fops.close = files[1]->fops.close = pipe_close;

    files[0]->uid = files[1]->uid = task_curr()->euid;
    files[0]->gid = files[1]->gid = task_curr()->egid;
    
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
    struct inode* files[2];
    int ret = pipe_create(files);

    if (ret) return ret;

    if (find_slots(fds, 2)) return -EMFILE;

    struct file* fd1 = kcalloc(sizeof(struct file));
    struct file* fd2 = kcalloc(sizeof(struct file));

    fd1->inode = files[0];
    fd1->off   = 0;
    fd1->ops   = files[0]->fops;
    fd1->flags = O_RDONLY;

    fd2->inode = files[1];
    fd2->off   = 0;
    fd2->ops   = files[1]->fops;
    fd2->flags = O_WRONLY;

    task_curr()->fds[fds[0]] = fd1;
    task_curr()->fds[fds[1]] = fd2;

    return 0;
}
