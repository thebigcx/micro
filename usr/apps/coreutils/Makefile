SRCS := cat.c pwd.c
OBJS := $(patsubst %.c, %.o, $(SRCS))
TARGS := $(patsubst %.c, %, $(SRCS))
LDFLAGS := -Tlinker.ld -static -Bsymbolic -nostdlib -L$(ROOT)/usr/lib -l:libc.a
CFLAGS := -ffreestanding -fshort-wchar -mno-red-zone -Wall -nostdinc -I$(ROOT)/usr/include -fno-stack-protector

.PHONY: all install

all: cat pwd install

# TODO: this system is bad
cat: cat.c
	@echo "    CC     cat.o"
	@gcc $(CFLAGS) -c cat.c -o cat.o
	@echo "    LD     cat"
	@ld -o cat cat.o $(LDFLAGS)

pwd: pwd.c
	@echo "    CC     pwd.o"
	@gcc $(CFLAGS) -c pwd.c -o pwd.o
	@echo "    LD     pwd"
	@ld -o pwd pwd.o $(LDFLAGS)

#install:
#	cp cat ../../../base/usr/bin/
#	cp ls ../../../base/usr/bin/

install:
	@mkdir -p $(ROOT)/usr/bin
	@cp cat $(ROOT)/usr/bin
	@cp pwd $(ROOT)/usr/bin

clean:
	@echo "    RM     $(TARGS)"
	@rm -f $(TARGS) $(OBJS)
