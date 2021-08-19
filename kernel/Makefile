SRC = src/arch/x86_64/boot/entry.c     	\
	  src/arch/x86_64/gdt.c             \
	  src/arch/x86_64/idt.c             \
      src/debug/serial.c                \
	  src/util/stdlib.c                 \
	  src/init.c

ASM = src/arch/x86_64/gdt_asm.S         \
	  src/arch/x86_64/idt_asm.S

OBJ = $(patsubst %.c, %.o, $(SRC)) $(patsubst %.S, %.o, $(ASM))
TARG = dist/vmkernel

INCLUDES = -Isrc -Isrc/arch/x86_64
CFLAGS = -ffreestanding -fno-stack-protector -g -Wall -Wextra -Wno-unused-function -fno-pic -fpie -mno-red-zone $(INCLUDES) -DDEBUG
LDFLAGS = -Tkernel.ld -static -pie -nostdlib

all: $(TARG)

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
