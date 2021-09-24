#pragma once

#include <micro/types.h>
#include <micro/list.h>

struct regs;
struct task;
struct thread;

struct sem
{
    struct list threads;
    size_t      max;
    size_t      cnt;
};

void sched_init();
void sched_tick(struct regs* r);
void switch_task(struct regs* r);
void switch_next();
void sched_start(struct task* task);
void sched_yield();

void sched_spawnthread(struct thread* thread);

struct task* sched_task_fromid(int id);

// sem.c - Semaphore functionality
struct sem* sem_create(size_t max);
void sem_signal(struct sem* sem);
void sem_wait(struct sem* sem);