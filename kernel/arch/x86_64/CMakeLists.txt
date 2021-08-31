set(SOURCES ${SOURCES} ${ARCHSRC}/x86_64/mmu.c
                       ${ARCHSRC}/x86_64/idt.c
                       ${ARCHSRC}/x86_64/gdt.c
                       ${ARCHSRC}/x86_64/cpu.c
                       ${ARCHSRC}/x86_64/boot/entry.c
                       ${ARCHSRC}/x86_64/intr_asm.S
                       ${ARCHSRC}/x86_64/x86_64.S
                       ${ARCHSRC}/x86_64/lapic.c
                       ${ARCHSRC}/x86_64/ioapic.c
                       ${ARCHSRC}/x86_64/pic.c
                       ${ARCHSRC}/x86_64/smp.c
                       ${ARCHSRC}/x86_64/ap_start.S
                       ${ARCHSRC}/x86_64/context.S
                       ${ARCHSRC}/x86_64/timer.c
                       ${ARCHSRC}/x86_64/except.c
                       ${ARCHSRC}/x86_64/panic.c
                       PARENT_SCOPE)

message(${ARCHSRC}/include)
include_directories(${ARCHSRC}/include PARENT_SCOPE)
