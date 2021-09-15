OBJ = $(patsubst %.c, %.ko, $(SRC))

CFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -mno-red-zone -nostdinc -I../../include -I../../arch/amd64/include -I../../include/uapi -static -nostdlib -mcmodel=large

all: $(TARG)

.PHONY: clean all install

%.ko: %.c
	@echo "    CC $@"
	@x86_64-micro-gcc -c $< -o $@ $(CFLAGS)

install:
	mkdir -p $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/lib/modules
	cp $(TARG) $(DESTDIR)/lib/modules/

clean:
	@echo "    RM $(TARG)"
	@rm -f $(OBJ) $(TARG)