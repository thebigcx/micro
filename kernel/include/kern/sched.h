#pragma once

struct regs;
struct task;

void sched_init();
void sched_tick(struct regs* r);
void switch_task(struct regs* r);
void switch_next();
void sched_start(struct task* task);
void sched_yield();