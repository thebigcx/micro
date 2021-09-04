LDFLAGS = -Tkernel.ld -nostdlib
CFLAGS = -Wall -Wextra -ffreestanding -g -DDEBUG -fno-stack-protector -fno-pic -fpie -mno-red-zone -nostdinc -fsanitize=undefined -Iinclude -Iinclude/uapi

include arch/amd64/amd64.mk
include kern/kern.mk

OBJ = $(patsubst %.c, %.o, $(SRC)) $(patsubst %.S, %.o, $(ASM))
TARGET = dist/vmkernel

all: $(TARGET) dev

.PHONY: clean dev

$(TARGET): $(OBJ)
	@echo "    LD $@"
	@mkdir -p dist
	@gcc $^ -o $@ $(LDFLAGS)

dev:
	$(MAKE) -C dev

%.o: %.c
	@echo "    CC $@"
	@gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "    AS $@"
	@gcc $(CFLAGS) -c $< -o $@

clean:
	@echo "    RM $(TARGET)"
	@rm -f $(OBJ) $(TARGET)