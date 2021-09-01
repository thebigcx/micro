SOURCES := init.c
ASM := syscall.S

OBJS := $(patsubst %.c, %.o, $(SOURCES)) $(patsubst %.S, %.o, $(ASM))
TARG = init
CFLAGS := -ffreestanding -fno-stack-protector -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -nostdinc -I$(ROOT)/usr/include

all: $(TARG)

$(TARG): $(OBJS)
	@echo "    LD      $@"
	@ld -Tlinker.ld -static -nostdlib -Bsymbolic -o $@ $^ -L$(ROOT)/usr/lib -l:libc.a

%.o: %.c
	@echo "    CC      $@"
	@gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "    AS      $@"
	@gcc -c $< -o $@

.PHONY:
clean:
	@echo "    RM      $(TARG)"
	@rm -f $(OBJS) $(TARG)