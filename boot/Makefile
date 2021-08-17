SRC = 
ASM = src/bootsect.S
OBJ = $(patsubst %.c, %.o, $(SRC)) $(patsubst %.S, %.o, $(ASM))
BIN = boot.bin
TARG = boot.img

CFLAGS = -ffreestanding
LDFLAGS = -Tlinker.ld --oformat binary

$(TARG): $(BIN)
	@echo "building boot image..."
	tools/mkbootsect $<
	@rm $<

$(BIN): $(OBJ)
	@echo "    LD $@"
	@ld $(LDFLAGS) -o $@ $^

%.o: %.S
	@echo "    CC $@"
	@gcc $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	@echo "    RM $(TARG)"
	@rm $(TARG) $(OBJ)
