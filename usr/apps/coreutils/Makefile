SRCS := cat.c pwd.c ls.c touch.c mkdir.c insmod.c rmmod.c
OBJS := $(patsubst %.c, %.o, $(SRCS))
TARGS := $(patsubst %.c, %, $(SRCS))
CFLAGS := -ffreestanding -fshort-wchar -mno-red-zone -Wall -fno-stack-protector

CC = x86_64-micro-gcc

.PHONY: all install clean

all: $(TARGS)

%: %.c
	@echo "    CC    $@.o"
	@$(CC) $(CFLAGS) -c $< -o $@.o
	@echo "    LD    $@"
	@$(CC) -static -o $@ $@.o

install:
	@mkdir -p $(DESTDIR)/usr/bin
	@cp cat $(DESTDIR)/usr/bin
	@cp pwd $(DESTDIR)/usr/bin
	@cp ls $(DESTDIR)/usr/bin
	@cp touch $(DESTDIR)/usr/bin
	@cp mkdir $(DESTDIR)/usr/bin
	@cp insmod $(DESTDIR)/usr/bin
	@cp rmmod $(DESTDIR)/usr/bin

clean:
	@echo "    RM    $(TARGS)"
	@rm -f $(TARGS) $(OBJS)