OBJ = $(patsubst %.c, %.o, $(SRC))

CFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -O3 -mno-red-zone -nostdinc -I../../include -I../../arch/amd64/include -I../../include/uapi -mcmodel=large
LDFLAGS = -static -nostdlib -Wl,-relocatable

all: $(TARG)

.PHONY: clean all install

$(TARG): $(OBJ)
	@echo "    CC $@"
	@x86_64-micro-gcc $^ -o $@ $(LDFLAGS)

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