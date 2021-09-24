#include <micro/sched.h>
#include <micro/thread.h>
#include <micro/heap.h>

struct sem* sem_create(size_t max)
{
    struct sem* sem = kmalloc(sizeof(struct sem));

    sem->threads = list_create();
    sem->max     = max;
    sem->cnt     = 0;

    return sem;
}

// Signal a semaphore - decrement its value
void sem_signal(struct sem* sem)
{
    if (sem->threads.size)
    {
        // Wake up the first waiting thread
        struct thread* t = list_dequeue(&sem->threads);
        sched_spawnthread(t);
    }
    else
        sem->cnt--; // No waiting threads
}

// Wait on a sempahore - increment its value
void sem_wait(struct sem* sem)
{
    if (sem->cnt < sem->max)
        sem->cnt++;
    else
    {
        // Wait for a thread to signal the semaphore
        list_push_back(&sem->threads, thread_curr());
        thread_block();
    }
}