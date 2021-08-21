#pragma once

#define DELIV_FIXED  0x000
#define DELIV_LOWEST 0x100
#define DELIV_SMI    0x200
#define DELIV_NMI    0x400
#define DELIV_INIT   0x500
#define DELIV_STRT   0x600

#define DST_SELF   0x40000
#define DST_ALL    0x80000
#define DST_OTHERS 0xc0000

void lapic_eoi();
void lapic_setup();
void lapic_enable();
