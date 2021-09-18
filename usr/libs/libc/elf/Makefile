SRC = ld.c
OBJ = $(patsubst %.c, %.o, $(SRC))

TARG = ld.so

CFLAGS = -ffreestanding -fno-stack-protector -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -static
CC = x86_64-micro-gcc

.PHONY: all install clean

all: $(TARG)

$(TARG): $(OBJ)
	@echo "    LD $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	@echo "    CC $@"
	@$(CC) -c $^ -o $@ $(CFLAGS)

install:
	cp -f $(TARG) $(DESTDIR)/lib/

clean:
	@echo "    RM $(TARG)"
	@rm -f $(TARG) $(OBJ)