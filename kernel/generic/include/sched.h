#pragma once

struct regs;

void sched_init();
void sched_tick(struct regs* r);
void switch_next(struct regs* r);
