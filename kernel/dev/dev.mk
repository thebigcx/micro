OBJ = $(patsubst %.c, %.ko, $(SRC))

CFLAGS = -Wall -Wextra -ffreestanding -fno-stack-protector -mno-red-zone -nostdinc -I../../include -I../include/uapi
LDFLAGS = -T../linker.ld -static -Bsymbolic

all: $(TARG)

.PHONY: clean all

#$(TARG): $(OBJ)
#	@echo "    AR $@"
#	@ar rvs $@ $^

%.ko: %.c
	@echo "    CC $@"
	@gcc -c $< -o $@ $(CFLAGS)

clean:
	@echo "    RM $(TARG)"
	@rm -f $(OBJ) $(TARG)