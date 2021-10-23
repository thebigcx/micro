SOURCES := db.c

OBJS := $(patsubst %.c, %.o, $(SOURCES))
TARG = db
CFLAGS := -O3 -ffreestanding -fno-stack-protector -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable

CC = x86_64-micro-gcc

all: $(TARG)

$(TARG): $(OBJS)
	@echo "    LD    $@"
	@$(CC) -static -o $@ $^

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