CFLAGS = -ffreestanding
LDFLAGS = -Tlinker.ld --oformat binary

.PHONY: all clean

all: bootsect.bin

bootsect.bin: bootsect.S
	@echo "    AS bootsect.o"
	@gcc $(CFLAGS) -c bootsect.S -o bootsect.o
	@echo "    LD $@"
	@ld $(LDFLAGS) -o bootsect.bin bootsect.o

%.o: %.S
	@echo "    AS $@"
	@gcc $(CFLAGS) -o $@ -c $<

clean:
	@echo "    RM bootsect.bin"
	@rm -f bootsect.bin bootsect.o