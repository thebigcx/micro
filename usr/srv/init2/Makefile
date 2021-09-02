SOURCES := init2.c

OBJS := $(patsubst %.c, %.o, $(SOURCES))
TARG = init2
CFLAGS := -ffreestanding -fno-stack-protector -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -nostdinc -I$(ROOT)/usr/include

all: $(TARG)

$(TARG): $(OBJS)
	@echo "    LD      $@"
	@ld -Tlinker.ld -static -nostdlib -Bsymbolic -o $@ $^ -L$(ROOT)/usr/lib -l:libc.a

%.o: %.c
	@echo "    CC      $@"
	@gcc $(CFLAGS) -c $< -o $@

.PHONY:
clean:
	@echo "    RM      $(TARG)"
	@rm -f $(OBJS) $(TARG)