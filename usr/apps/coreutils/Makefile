SRCS := cat.c 		\
		pwd.c 		\
		ls.c 		\
		touch.c 	\
		mkdir.c 	\
		insmod.c 	\
		rmmod.c 	\
		mount.c 	\
		umount.c	\
		date.c		\
		du.c		\
		stat.c		\
		rm.c		\
		echo.c		\
		chmod.c		\
		chown.c		\
		readlink.c	\
		id.c		\
		whoami.c	\
		ln.c		\
		hexdump.c	\
		tee.c		\
		mv.c		\
		rmdir.c		\
		uname.c

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
	$(foreach targ, $(TARGS), cp $(targ) $(DESTDIR)/usr/bin/;)

clean:
	@echo "    RM    $(TARGS)"
	@rm -f $(TARGS) $(OBJS)