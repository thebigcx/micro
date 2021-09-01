#pragma once

struct regs;
struct task;

void sched_init();
void sched_tick(struct regs* r);
void switch_next(struct regs* r);
void sched_start(struct task* task);
