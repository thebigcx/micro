SRC = src/arch/x86_64/boot/entry.c     \
      src/debug/serial.c

OBJ = $(patsubst %.c, %.o, $(SRC))
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

clean:
	@echo "cleaning..."
	@rm -f $(OBJ) $(TARG)
