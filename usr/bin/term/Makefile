SOURCES := term.c

OBJS := $(patsubst %.c, %.o, $(SOURCES))
TARG = term
CFLAGS := -O3 -ffreestanding -fno-stack-protector -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS := -lansi

CC = x86_64-micro-gcc

all: $(TARG)

$(TARG): $(OBJS)
	@echo "    LD    $@"
	@$(CC) -static -o $@ $^ $(LDFLAGS)

%.o: %.c
	@echo "    CC    $@"
	@$(CC) $(CFLAGS) -c $< -o $@

install:
	mkdir -p $(DESTDIR)/usr
	mkdir -p $(DESTDIR)/usr/bin
	cp $(TARG) $(DESTDIR)/usr/bin

.PHONY:
clean:
	@echo "    RM    $(TARG)"
	@rm -f $(OBJS) $(TARG)
