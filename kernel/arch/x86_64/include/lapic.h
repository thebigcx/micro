#pragma once

#include <types.h>

#define DELIV_FIXED  0x00000
#define DELIV_LOWEST 0x00100
#define DELIV_SMI    0x00200
#define DELIV_NMI    0x00400
#define DELIV_INIT   0x00500
#define DELIV_STRT   0x00600

#define DSTMODE_PHYS 0x00000
#define DSTMODE_LOG  0x01000

#define LVL_DEASSRT  0x00000
#define LVL_ASSRT    0x04000

#define TRIG_EDGE    0x00000
#define TRIG_LVL     0x00001

#define DST_SELF     0x40000
#define DST_ALL      0x80000
#define DST_OTHERS   0xc0000

void lapic_send_ipi(unsigned int id, uint8_t vec, uint32_t flags);
void lapic_eoi();
void lapic_setup();
void lapic_enable();
