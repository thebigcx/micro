SOURCES := init.c

OBJS := $(patsubst %.c, %.o, $(SOURCES))
TARG = init
CFLAGS := -fno-stack-protector -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable 

all: $(TARG)

$(TARG): $(OBJS)
	@echo "    LD    $@"
	@x86_64-micro-gcc -static -o $@ $^

%.o: %.c
	@echo "    CC    $@"
	@x86_64-micro-gcc $(CFLAGS) -c $< -o $@

install:
	cp $(TARG) $(DESTDIR)/

.PHONY:
clean:
	@echo "    RM    $(TARG)"
	@rm -f $(OBJS) $(TARG)
