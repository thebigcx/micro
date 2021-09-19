#pragma once

#include <micro/types.h>

struct rtc_time
{
    uint8_t  sec;
    uint8_t  min;
    uint8_t  hour;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
};

void rtc_gettime(struct rtc_time* time);
void rtc_init();