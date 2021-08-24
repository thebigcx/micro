#include <sched.h>
#include <cpu.h>
#include <lapic.h>
#include <debug/syslog.h>

static int ready;

void sched_init()
{
    ready = 1;
    for(;;);
}

void switch_next(struct regs* r)
{
    dbgln("sched()");

    struct cpu_info* cpu = cpu_curr();
}

void sched_tick(struct regs* r)
{
    if (!ready) return;
    lapic_send_ipi(0, IPI_SCHED, DST_OTHERS | DELIV_FIXED);
    switch_next(r);
}
