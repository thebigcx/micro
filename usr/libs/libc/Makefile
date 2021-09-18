SOURCES := internal/generic/init.c				\
		   internal/generic/crt0.c  			\
		   ansi/generic/malloc.c				\
		   ansi/generic/math.c					\
		   ansi/generic/printf.c				\
		   ansi/generic/stdio.c					\
		   ansi/generic/stdlib.c  				\
		   ansi/generic/convert.c				\
		   ansi/generic/rand.c					\
		   ansi/generic/string.c				\
		   ansi/generic/signal.c				\
		   ansi/generic/errno.c					\
		   ansi/generic/assert.c				\
		   ansi/generic/ctype.c 				\
		   ansi/generic/time.c	 				\
		   posix/generic/openpty.c				\
		   posix/generic/opendir.c				\
		   posix/generic/closedir.c				\
		   posix/generic/isatty.c				\
		   posix/generic/execv.c				\
		   posix/generic/execvp.c				\
		   sysdeps/micro/generic/micro.c		\
		   sysdeps/micro/generic/open.c			\
		   sysdeps/micro/generic/close.c		\
		   sysdeps/micro/generic/read.c			\
		   sysdeps/micro/generic/write.c		\
		   sysdeps/micro/generic/pread.c		\
		   sysdeps/micro/generic/pwrite.c		\
		   sysdeps/micro/generic/access.c		\
		   sysdeps/micro/generic/lseek.c		\
		   sysdeps/micro/generic/ioctl.c		\
		   sysdeps/micro/generic/readdir.c		\
		   sysdeps/micro/generic/mkdir.c		\
		   sysdeps/micro/generic/truncate.c		\
		   sysdeps/micro/generic/ftruncate.c	\
		   sysdeps/micro/generic/dup.c			\
		   sysdeps/micro/generic/dup2.c			\
		   sysdeps/micro/generic/mount.c		\
		   sysdeps/micro/generic/umount.c		\
		   sysdeps/micro/generic/getpid.c		\
		   sysdeps/micro/generic/fork.c			\
		   sysdeps/micro/generic/execve.c		\
		   sysdeps/micro/generic/waitpid.c		\
		   sysdeps/micro/generic/chdir.c		\
		   sysdeps/micro/generic/getcwd.c		\
		   sysdeps/micro/generic/mmap.c			\
		   sysdeps/micro/generic/munmap.c		\
		   sysdeps/micro/generic/sleep.c		\
		   sysdeps/micro/generic/tcsetattr.c	\
		   sysdeps/micro/generic/tcgetattr.c	\

ASM :=     sysdeps/micro/generic/syscall.S

OBJS := $(patsubst %.c, %.o, $(SOURCES)) $(patsubst %.S, %.o, $(ASM))
AR := libc.a
SO := libc.so
CRT0 := crt0.o
CFLAGS := -fPIC -fno-omit-frame-pointer -ffreestanding -fno-stack-protector -O3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Iansi/include -Iinternal/include -Iposix/include -Isysdeps/micro/include

all: $(AR) $(SO) $(CRT0)

$(CRT0):
	@cp internal/generic/crt0.o .

$(AR): $(OBJS)
	@echo "    AR    $@"
	@x86_64-micro-ar rcs $@ $^

$(SO): $(OBJS)
	@echo "    LD    $@"
	@x86_64-micro-gcc -Wl,-Bsymbolic -shared -nostdlib -o $@ $^

%.o: %.c
	@echo "    CC    $@"
	@x86_64-micro-gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "    AS    $@"
	@x86_64-micro-gcc -c $< -o $@

#SUBDIRS = elf

#include subdir.mk

install:
	mkdir -p $(DESTDIR)/usr
	mkdir -p $(DESTDIR)/usr/include
	mkdir -p $(DESTDIR)/usr/lib
	cp $(AR) $(DESTDIR)/usr/lib/
	cp $(SO) $(DESTDIR)/usr/lib/
	cp $(CRT0) $(DESTDIR)/usr/lib/
	cp -r ansi/include/* $(DESTDIR)/usr/include/
	cp -r posix/include/* $(DESTDIR)/usr/include/

.PHONY:
clean:
	@echo "    RM    $(SO) $(AR)"
	@rm -f $(OBJS) $(SO) $(AR) $(CRT0)