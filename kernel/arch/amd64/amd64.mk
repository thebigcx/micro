SRC +=  arch/amd64/amd64/boot/entry.c 	\
		arch/amd64/amd64/cpu.c 			\
		arch/amd64/amd64/except.c 		\
		arch/amd64/amd64/gdt.c 			\
		arch/amd64/amd64/idt.c 			\
		arch/amd64/amd64/ioapic.c 		\
		arch/amd64/amd64/lapic.c 		\
		arch/amd64/amd64/mmu.c 			\
		arch/amd64/amd64/panic.c 		\
		arch/amd64/amd64/pic.c 			\
		arch/amd64/amd64/smp.c 			\
		arch/amd64/amd64/timer.c

ASM +=  arch/amd64/amd64/ap_start.S  	\
		arch/amd64/amd64/context.S  	\
		arch/amd64/amd64/intr_asm.S 	\
		arch/amd64/amd64/amd64.S

CFLAGS += -Iarch/amd64/include