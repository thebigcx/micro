LDFLAGS = -Tkernel.ld -nostdlib
CFLAGS = -Wall -Wextra -ffreestanding -g -DDEBUG -fno-stack-protector -fno-pic -fpie -mno-red-zone -nostdinc -fsanitize=undefined -Iinclude/kern

include arch/amd64/amd64.mk
include kern/kern.mk

OBJ = $(patsubst %.c, %.o, $(SRC)) $(patsubst %.S, %.o, $(ASM))
TARGET = dist/vmkernel

.PHONY: clean

$(TARGET): $(OBJ)
	@echo "    LD $@"
	@mkdir -p dist
	@gcc $^ -o $@ $(LDFLAGS)

%.o: %.c
	@echo "    CC $@"
	@gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "    AS $@"
	@gcc $(CFLAGS) -c $< -o $@

clean:
	@echo "    RM $(TARGET)"
	@rm -f $(OBJ) $(TARGET)