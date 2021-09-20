#include <arch/cmos.h>
#include <arch/pio.h>
#include <arch/descs.h>
#include <arch/reg.h>
#include <arch/lapic.h>
#include <arch/ioapic.h>
#include <arch/cpu_func.h>
#include <arch/timer.h>

#include <micro/sys.h>
#include <micro/time.h>

#define CMOS_SEC      0x00
#define CMOS_MIN      0x02
#define CMOS_HOUR     0x04
#define CMOS_WEEKDAY  0x06
#define CMOS_MONTHDAY 0x07
#define CMOS_MONTH    0x08
#define CMOS_YEAR     0x09
#define CMOS_CENTURY  0x32

#define CMOS_REG      0x70
#define CMOS_VAL      0x71

#define BCD2BIN(v) (v = (v & 0x0f) + ((v / 16) * 10))

uint8_t cmos_read(uint8_t reg)
{
    outb(CMOS_REG, (1 << 7) | reg); // Make sure to keep NMI enabled
    return inb(CMOS_VAL);
}

void cmos_write(uint8_t reg, uint8_t val)
{
    outb(CMOS_REG, (1 << 7) | reg);
    outb(CMOS_VAL, val);
}

int cmos_isupdate()
{
    return cmos_read(0x0a) & 0x80;
}

void rtc_gettime(struct rtc_time* time)
{
    while (cmos_isupdate());

    time->sec      = cmos_read(CMOS_SEC);
    time->min      = cmos_read(CMOS_MIN);
    time->hour     = cmos_read(CMOS_HOUR);
    time->day      = cmos_read(CMOS_MONTHDAY);
    time->month    = cmos_read(CMOS_MONTH);
    time->year     = cmos_read(CMOS_YEAR);

    uint8_t century = cmos_read(CMOS_CENTURY);

    uint8_t regb = cmos_read(0x0b);

    // Convert BCD to binary
    if (!(regb & 0x04))
    {
        BCD2BIN(time->sec);
        BCD2BIN(time->min);
        BCD2BIN(time->day);
        BCD2BIN(time->month);
        BCD2BIN(time->year);

        BCD2BIN(century);

        time->hour = ((time->hour & 0x0f) + (((time->hour & 0x70) / 16) * 10)) | (time->hour & 0x80);
    }

    // Convert to 24 hour clock
    if (!(regb & 0x02) && (time->hour & 0x80))
    {
        time->hour = ((time->hour & 0x7f) + 12) % 24;
    }

    // Full 4-digit year
    time->year += century * 100;
}

void rtc_handler(struct regs* r)
{
    struct rtc_time time;
    rtc_gettime(&time);

    cmos_read(0xc);
}

void rtc_init()
{
    idt_set_handler(8, rtc_handler);
    ioapic_redir(1, 33, DELIV_LOWEST);

    cli();
    cmos_write(0x8b, cmos_read(0x8b) | 0x40);
    sti();
}

// This is very basic and doesn't account for many things
time_t date_to_epoch(struct rtc_time* time)
{
    time_t sec = 0;

    sec += time->sec   * 1;
    sec += time->min   * 60;
    sec += time->hour  * 3600;
    sec += time->day   * 86400;
    sec += time->month * 2592000;

    sec += (time->year - 1970)  * 31104000;

    return sec;
}

SYSCALL_DEFINE(gettimeofday, struct timeval* tv, struct timezone* tz)
{
    PTRVALID(tv);
    PTRVALID(tz);

    struct rtc_time time;
    rtc_gettime(&time);

    tv->tv_sec  = date_to_epoch(&time);
    tv->tv_usec = timer_usec();

    tz->tz_dsttime = 0;
    tz->tz_minuteswest = 0;

    return 0;
}

SYSCALL_DEFINE(time, time_t* time)
{
    PTRVALID(time);

    struct rtc_time rtc;
    rtc_gettime(&rtc);

    *time = date_to_epoch(&rtc);

    return 0;
}