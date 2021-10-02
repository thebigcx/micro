LDFLAGS = -Tkernel.ld -nostdlib -zmax-page-size=0x1000 -static -pie --no-dynamic-linker -ztext
CFLAGS = -Wall -Wextra -Wno-unused-parameter -ffreestanding -fno-stack-protector -fno-pic -fpie -mno-red-zone -nostdinc -Iinclude -Iinclude/uapi

include arch/amd64/amd64.mk
include kern/kern.mk

OBJ = $(patsubst %.c, %.o, $(SRC)) $(patsubst %.S, %.o, $(ASM))
TARGET = dist/vmkernel

release: CFLAGS += -Ofast
release: $(TARGET) dev

debug: CFLAGS += -g# -DDEBUG -fsanitize=undefined -fsanitize=kernel-address
debug: $(TARGET) dev

.PHONY: clean dev install

$(TARGET): $(OBJ)
	@echo "    LD $@"
	@mkdir -p dist
	@ld $^ -o $@ $(LDFLAGS)

dev:
	$(MAKE) -C dev

%.o: %.c
	@echo "    CC $@"
	@gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "    AS $@"
	@gcc $(CFLAGS) -c $< -o $@

install:
	cp -r include/uapi/* $(DESTDIR)/usr/include/
	$(MAKE) -C dev install

clean:
	@echo "    RM $(TARGET)"
	@rm -f $(OBJ) $(TARGET)
	$(MAKE) -C dev clean
