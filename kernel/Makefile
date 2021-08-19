SRC = src/arch/x86_64/boot/entry.c     	\
	  src/arch/x86_64/gdt.c             \
	  src/arch/x86_64/idt.c             \
	  src/arch/x86_64/mmu.c             \
      src/debug/serial.c                \
	  src/debug/ubsan.c	                \
	  src/util/stdlib.c                 \
	  src/init.c                        \
	  src/acpi.c

ASM = src/arch/x86_64/asm.S             \
	  src/arch/x86_64/intr_asm.S

OBJ = $(patsubst %.c, %.o, $(SRC)) $(patsubst %.S, %.o, $(ASM))
TARG = dist/vmkernel

INCLUDES = -Isrc -Isrc/arch/x86_64
CFLAGS = -ffreestanding -fno-stack-protector -Wall -Wextra -Wno-unused-function -fno-pic -fpie -mno-red-zone $(INCLUDES)
LDFLAGS = -Tkernel.ld -static -pie -nostdlib

.PHONY: all debug clean

all: $(TARG)

debug: CFLAGS += -g -DDEBUG -fsanitize=undefined
debug: $(TARG)

$(TARG): $(OBJ)
	@mkdir -p dist
	@echo "    LD $@"
	@ld -o $@ $^ $(LDFLAGS)

%.o: %.c
	@echo "    CC $@"
	@gcc -c $< -o $@ $(CFLAGS)

%.o: %.S
	@echo "    AS $@"
	@gcc -c $< -o $@ $(CFLAGS)

clean:
	@echo "cleaning..."
	@rm -f $(OBJ) $(TARG)
