#pragma once

#include <arch/mmu.h>
#include <micro/list.h>
#include <micro/signal.h>

#define FD_MAX 64

struct task
{
    unsigned int id;

    struct list threads;  // struct thread*[]
    struct list children; // struct task*[]
    struct task* parent;

    // Array of FD_MAX file descriptors
    struct fd** fds;

    struct thread* main;

    struct vm_map* vm_map;

    char workd[64]; // Working directory

    struct list sigqueue;  // struct int[]
    uintptr_t signals[32];
    uint32_t sigmask;

    int dead;
    int status;

    volatile int waiting;

    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;
};

struct thread;

struct task* task_idle();
struct task* task_init_creat();
struct task* task_kcreat(struct task* parent, uintptr_t entry);
struct task* task_clone(struct task* src, struct thread* calling);
struct task* task_curr();

void task_send(struct task* task, int signal);

void task_execve(struct task* task, const char* path, const char* argv[], const char* envp[]);

void task_exit(int val);
void task_delete(struct task* task);