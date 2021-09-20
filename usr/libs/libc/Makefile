SOURCES := internal/generic/init.c				\
		   internal/generic/crt0.c  			\
		   ctype/isalnum.c						\
		   ctype/isalpha.c						\
		   ctype/isdigit.c						\
		   ctype/isprint.c						\
		   ctype/ispunct.c						\
		   ctype/isspace.c						\
		   ctype/isxdigit.c						\
		   ctype/tolower.c						\
		   ctype/toupper.c						\
		   ctype/isgraph.c						\
		   assert/assert.c						\
		   string/memchr.c						\
		   string/memcmp.c						\
		   string/memcpy.c						\
		   string/memmove.c						\
		   string/memset.c						\
		   string/strcat.c						\
		   string/strchr.c						\
		   string/strcmp.c						\
		   string/strcpy.c						\
		   string/strcspn.c						\
		   string/strdup.c						\
		   string/strlen.c						\
		   string/strncmp.c						\
		   string/strncpy.c						\
		   string/strrchr.c						\
		   string/strrev.c						\
		   string/strsep.c						\
		   string/strspn.c						\
		   string/strstr.c						\
		   string/strtok_r.c					\
		   string/strtok.c						\
		   string/strerror.c					\
		   string/strndup.c						\
		   stdlib/abs.c							\
		   stdlib/atoi.c						\
		   stdlib/itoa.c						\
		   stdlib/qsort.c						\
		   stdlib/rand.c						\
		   stdlib/strtoul.c						\
		   stdlib/ultoa.c						\
		   stdlib/abort.c						\
		   stdlib/getenv.c						\
		   stdlib/atexit.c						\
		   stdio/fprintf.c						\
		   stdio/printf.c						\
		   stdio/snprintf.c						\
		   stdio/sprintf.c						\
		   stdio/vfprintf.c						\
		   stdio/vsnprintf.c					\
		   stdio/vsprintf.c						\
		   stdio/fclose.c						\
		   stdio/feof.c							\
		   stdio/ferror.c						\
		   stdio/fflush.c						\
		   stdio/fgetc.c						\
		   stdio/fgets.c						\
		   stdio/fileno.c						\
		   stdio/fopen.c						\
		   stdio/fputc.c						\
		   stdio/fputs.c						\
		   stdio/fread.c						\
		   stdio/fseek.c						\
		   stdio/ftell.c						\
		   stdio/fwrite.c						\
		   stdio/getline.c						\
		   stdio/perror.c						\
		   stdio/putchar.c						\
		   stdio/puts.c							\
		   stdio/remove.c						\
		   stdio/rewind.c						\
		   stdio/setbuf.c						\
		   stdio/setvbuf.c						\
		   stdio/sscanf.c						\
		   stdio/tmpfile.c						\
		   stdio/ungetc.c						\
		   stdio/stdio.c						\
		   stdio/freopen.c						\
		   stdio/vprintf.c						\
		   time/gmtime_r.c						\
		   time/gmtime.c						\
		   time/localtime_r.c					\
		   time/localtime.c						\
		   time/strftime.c						\
		   time/asctime.c						\
		   time/asctime_r.c						\
		   time/ctime.c							\
		   time/ctime_r.c						\
		   math/ceil.c							\
		   math/fabs.c							\
		   math/floor.c							\
		   math/fmod.c							\
		   math/log.c							\
		   math/pow.c							\
		   math/trunc.c							\
		   malloc/malloc.c						\
		   setjmp/setjmp.c						\
		   setjmp/longjmp.c						\
		   locale/nl_langinfo.c					\
		   misc/errno.c							\
		   posix/generic/openpty.c				\
		   posix/generic/opendir.c				\
		   posix/generic/closedir.c				\
		   posix/generic/isatty.c				\
		   posix/generic/execv.c				\
		   posix/generic/execvp.c				\
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
		   sysdeps/micro/generic/kill.c			\
		   sysdeps/micro/generic/raise.c		\
		   sysdeps/micro/generic/sigaction.c	\
		   sysdeps/micro/generic/signal.c		\
		   sysdeps/micro/generic/exit.c			\
		   sysdeps/micro/generic/ptsname.c		\
		   sysdeps/micro/generic/ptsname_r.c	\
		   sysdeps/micro/generic/nanosleep.c	\
		   sysdeps/micro/generic/time.c			\
		   sysdeps/micro/generic/gettimeofday.c \
		   sysdeps/micro/generic/ptrace.c 		\

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
	cp -r include/* $(DESTDIR)/usr/include/
	cp -r posix/include/* $(DESTDIR)/usr/include/

.PHONY:
clean:
	@echo "    RM    $(SO) $(AR)"
	@rm -f $(OBJS) $(SO) $(AR) $(CRT0)