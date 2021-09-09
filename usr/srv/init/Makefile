SOURCES := init.c

CC = x86_64-micro-gcc
OBJS := $(patsubst %.c, %.o, $(SOURCES))
TARG = init
CFLAGS := -ffreestanding -fno-stack-protector -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable

all: $(TARG)

$(TARG): $(OBJS)
	@echo "    LD    $@"
	@$(CC) -static -o $@ $^

%.o: %.c
	@echo "    CC    $@"
	@$(CC) $(CFLAGS) -c $< -o $@

install:
	cp $(TARG) $(DESTDIR)/

.PHONY:
clean:
	@echo "    RM    $(TARG)"
	@rm -f $(OBJS) $(TARG)
