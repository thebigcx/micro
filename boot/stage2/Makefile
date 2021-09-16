SRC = entry.c
OBJ = $(patsubst %.c, %.o, $(SRC))

CFLAGS = -ffreestanding -fno-stack-protector 
LDFLAGS = -Tlinker.ld -nostdlib -static

TARG = loader

.PHONY: all clean

all: $(TARG)

$(TARG): $(OBJ)
	@echo "    LD $@"
	@ld -o $@ $^ $(LDFLAGS)

%.o: %.c
	@echo "    CC $@"
	@gcc -o $@ -c $< $(CFLAGS)

clean:
	@echo "    RM $(TARG)"
	@rm $(TARG) $(OBJ)