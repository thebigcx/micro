SRC =
ASM = stage2/entry.S
OBJ = $(patsubst %.c, %.o, $(SRC)) $(patsubst %.S, %.o, $(ASM))

CFLAGS = -ffreestanding

.DEFAULT_GOAL := all

.PHONY: stage1 stage2 clean all

bootsect.bin: stage1/bootsect.S
	@echo "    AS stage1/bootsect.o"
	@gcc $(CFLAGS) -c stage1/bootsect.S -o stage1/bootsect.o
	@echo "    LD $@"
	@ld -Tstage1/linker.ld --oformat binary -o bootsect.bin stage1/bootsect.o

stage1: bootsect.bin

%.o: %.S
	@echo "    AS $@"
	@gcc $(CFLAGS) -o $@ -c $<

%.o: %.c
	@echo "    CC $@"
	@gcc $(CFLAGS) -o $@ -c $<

all:
	@$(MAKE) -s stage1
#	@$(MAKE) -s stage2
	@echo "building boot image..."
	@tools/mkbootsect bootsect.bin > /dev/null 2>&1

clean:
	@echo "cleaning..."
	@rm -f boot.img bootsect.bin stage1/bootsect.o $(OBJ) $(LOADER)
