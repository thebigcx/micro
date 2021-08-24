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
    struct cpu_info* cpu = cpu_curr();
    if (TEST_LOCK(cpu->lock)) return;

    if (cpu->current)
    {
        // add back
    }

    if (!cpu->ready.size)
    {
        // idle thread
    }

    UNLOCK(cpu->lock);

    // arch_switch_ctx(cpu->current);
}

void sched_tick(struct regs* r)
{
    if (!ready) return;
    lapic_send_ipi(0, IPI_SCHED, DST_OTHERS | DELIV_FIXED);
    switch_next(r);
}
