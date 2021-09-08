SRCS := cat.c pwd.c ls.c touch.c mkdir.c
OBJS := $(patsubst %.c, %.o, $(SRCS))
TARGS := $(patsubst %.c, %, $(SRCS))
LDFLAGS := -Tlinker.ld -static -Bsymbolic -nostdlib -L$(ROOT)/usr/lib -l:libc.a
CFLAGS := -ffreestanding -fshort-wchar -mno-red-zone -Wall -nostdinc -I$(ROOT)/usr/include -fno-stack-protector

.PHONY: all install

all: $(TARGS)

%: %.c
	@echo "    CC    $@.o"
	@gcc $(CFLAGS) -c $< -o $@.o
	@echo "    LD    $@"
	@ld -o $@ $@.o $(LDFLAGS)

install:
	@mkdir -p $(ROOT)/usr/bin
	@cp cat $(ROOT)/usr/bin
	@cp pwd $(ROOT)/usr/bin
	@cp ls $(ROOT)/usr/bin
	@cp touch $(ROOT)/usr/bin
	@cp mkdir $(ROOT)/usr/bin

clean:
	@echo "    RM    $(TARGS)"
	@rm -f $(TARGS) $(OBJS)