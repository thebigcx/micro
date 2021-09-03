SRCS := cat.c
OBJS := $(patsubst %.c, %.o, $(SRCS))
TARGS := $(patsubst %.c, %, $(SRCS))
LDFLAGS := -Tlinker.ld -static -Bsymbolic -nostdlib -L$(ROOT)/usr/lib -l:libc.a
CFLAGS := -ffreestanding -fshort-wchar -mno-red-zone -Wall -nostdinc -I$(ROOT)/usr/include -fno-stack-protector

all: cat

cat: cat.c
	@echo "    CC     cat.o"
	@gcc $(CFLAGS) -c cat.c -o cat.o
	@echo "    LD     cat"
	@ld -o cat cat.o $(LDFLAGS)

#install:
#	cp cat ../../../base/usr/bin/
#	cp ls ../../../base/usr/bin/

.PHONY:
clean:
	@echo "    RM     $(TARGS)"
	@rm -f $(TARGS) $(OBJS)
