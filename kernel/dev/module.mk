OBJ = $(patsubst %.c, %.o, $(SRC))

CFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -Ofast -fno-pic -mno-red-zone -nostdinc -I../../include -I../../arch/amd64/include -I../../include/uapi -mcmodel=large
LDFLAGS = -T../../kernel.ld -static -nostdlib

all: $(TARG)

.PHONY: clean all install

# TODO: multiple source files
$(TARG): $(OBJ)
	@echo "    LD $@"
	@x86_64-micro-ld -r $^ -o $@ $(LDFLAGS)

%.o: %.c
	@echo "    CC $@"
	@x86_64-micro-gcc -c $< -o $@ $(CFLAGS)

install:
	mkdir -p $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/lib/modules
	cp $(TARG) $(DESTDIR)/lib/modules/

clean:
	@echo "    RM $(TARG)"
	@rm -f $(OBJ) $(TARG)