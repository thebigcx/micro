SRC +=  arch/x86_64/x86_64/boot/entry.c \
		arch/x86_64/x86_64/cpu.c 		\
		arch/x86_64/x86_64/except.c 	\
		arch/x86_64/x86_64/gdt.c 		\
		arch/x86_64/x86_64/idt.c 		\
		arch/x86_64/x86_64/ioapic.c 	\
		arch/x86_64/x86_64/lapic.c 		\
		arch/x86_64/x86_64/mmu.c 		\
		arch/x86_64/x86_64/panic.c 		\
		arch/x86_64/x86_64/pic.c 		\
		arch/x86_64/x86_64/smp.c 		\
		arch/x86_64/x86_64/timer.c

ASM +=  arch/x86_64/x86_64/ap_start.S  	\
		arch/x86_64/x86_64/context.S  	\
		arch/x86_64/x86_64/intr_asm.S 	\
		arch/x86_64/x86_64/x86_64.S

CFLAGS += -Iarch/x86_64/include