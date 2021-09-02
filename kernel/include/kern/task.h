#pragma once

#include <mmu.h>
#include <list.h>
#include <signal.h>

struct task
{
    unsigned int id;

    struct list threads;  // struct thread*[]
    struct list fds;      // struct fd*[]
    struct list children; // struct task*[]
    struct task* parent;

    struct thread* main;

    struct vm_map* vm_map;

    struct list sigqueue;  // struct signal_t[]
    uintptr_t signals[32];
    uint32_t sigmask;
};

struct thread;

struct task* task_idle();
struct task* task_creat(struct task* parent, const void* buffer, char* argv[], char* envp[]);
struct task* task_kcreat(struct task* parent, uintptr_t entry);
struct task* task_clone(const struct task* src, struct thread* calling);
struct task* task_curr();

void task_send(struct task* task, signal_t signal);

void task_execve(struct task* task, const char* path, const char* argv[], const char* envp[]);

void task_destroy(struct task* task);