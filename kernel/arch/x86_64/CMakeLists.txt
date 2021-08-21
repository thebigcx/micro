set(SOURCES ${SOURCES} ${ARCHSRC}/src/mmu.c
                       ${ARCHSRC}/src/idt.c
                       ${ARCHSRC}/src/gdt.c
                       ${ARCHSRC}/src/boot/entry.c
                       ${ARCHSRC}/src/intr_asm.S
                       ${ARCHSRC}/src/asm.S
                       ${ARCHSRC}/src/lapic.c
                       ${ARCHSRC}/src/ioapic.c
                       PARENT_SCOPE)

message(${ARCHSRC}/include)
include_directories(${ARCHSRC}/include PARENT_SCOPE)
