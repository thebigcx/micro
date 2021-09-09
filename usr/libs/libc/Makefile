SOURCES := internal/generic/init.c				\
		   internal/generic/crt0.c  			\
		   ansi/generic/malloc.c				\
		   ansi/generic/math.c					\
		   ansi/generic/file.c					\
		   ansi/generic/printf.c				\
		   ansi/generic/stdio.c					\
		   ansi/generic/stdlib.c  				\
		   ansi/generic/convert.c				\
		   ansi/generic/rand.c					\
		   ansi/generic/string.c				\
		   ansi/generic/signal.c				\
		   ansi/generic/errno.c					\
		   ansi/generic/assert.c				\
		   posix/generic/file-io.c				\
		   posix/generic/posix-proc.c 			\
		   posix/generic/posix-time.c			\
		   posix/generic/mman.c					\
		   sysdeps/micro/generic/micro.c		\

ASM :=     sysdeps/micro/generic/syscall.S

OBJS := $(patsubst %.c, %.o, $(SOURCES)) $(patsubst %.S, %.o, $(ASM))
AR := libc.a
SO := libc.so
CRT0 := crt0.o
CFLAGS := -mno-sse -mno-sse2  -ffreestanding -fno-stack-protector -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Iansi/include -Iinternal/include -Iposix/include -Isysdeps/micro/include

all: $(AR) $(SO) $(CRT0)

$(CRT0):
	@cp internal/generic/crt0.o .

$(AR): $(OBJS)
	@echo "    AR    $@"
	@x86_64-micro-ar rcs $@ $^

$(SO): $(OBJS)
	@echo "    LD    $@"
	@x86_64-micro-ld -shared -Bsymbolic -o $@ $^

%.o: %.c
	@echo "    CC    $@"
	@gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo "    AS    $@"
	@gcc -c $< -o $@

install:
	mkdir -p $(DESTDIR)/usr
	mkdir -p $(DESTDIR)/usr/include
	mkdir -p $(DESTDIR)/usr/lib
	cp $(AR) $(DESTDIR)/usr/lib/
	cp $(SO) $(DESTDIR)/usr/lib/
	cp $(CRT0) $(DESTDIR)/usr/lib/
	cp -r ansi/include/* $(DESTDIR)/usr/include/
	cp -r posix/include/* $(DESTDIR)/usr/include/
	cp -r sysdeps/micro/include/* $(DESTDIR)/usr/include/

.PHONY:
clean:
	@echo "    RM    $(SO) $(AR)"
	@rm -f $(OBJS) $(SO) $(AR) $(CRT0)
