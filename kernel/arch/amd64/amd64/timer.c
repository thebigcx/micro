#include <arch/timer.h>
#include <arch/descs.h>
#include <arch/cpu.h>
#include <micro/sched.h>
#include <arch/pio.h>

#define FREQ 1193182

#define CHAN0       0x40
#define CHAN1       0x41
#define CHAN2       0x42
#define CMD         0x43

#define CMD_BIN     0x00
#define CMD_BCD     0x01

#define CMD_MODE0   0x00
#define CMD_MODE1   0x02
#define CMD_MODE2   0x04
#define CMD_MODE3   0x06
#define CMD_MODE4   0x08
#define CMD_MODE5   0x0a

#define CMD_LATCH   0x00
#define CMD_RW_LOW  0x10
#define CMD_RW_HI   0x20
#define CMD_RW_BOTH 0x30

#define CMD_CHAN0   0x00
#define CMD_CHAN1   0x40
#define CMD_CHAN2   0x80
#define CMD_RBACK   0xc0

static volatile uint64_t uptime = 0;
static uint64_t freq;

void timer_tick(struct regs* r)
{
    uptime += 1000000000 / freq;
    sched_tick(r);
}

void timer_wait(uint64_t ns)
{
    uint64_t now = uptime;
    while (uptime - now < ns);
}

void timer_init()
{
    idt_set_handler(32, timer_tick);

    freq = 100;
    uint32_t div = FREQ / freq;

    outb(CMD, CMD_BIN | CMD_MODE3 | CMD_RW_BOTH | CMD_CHAN0);

    outb(CHAN0, div & 0xff);
    outb(CHAN0, div >> 8);   
}
