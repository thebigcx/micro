SRC = ansi.c
OBJ = $(patsubst %.c, %.o, $(SRC))

LIBSO = libansi.so
LIBAR = libansi.a

CFLAGS = -I. -fPIC -fno-stack-protector

CC = x86_64-micro-gcc
AR = x86_64-micro-ar

.PHONY: all clean install

all: $(LIBAR) #$(LIBSO)
	
$(LIBSO): $(OBJ)
	@echo "    LD    $@"
	@$(CC) -o $@ $^ -shared -Wl,-Bsymbolic $(CFLAGS)
	
$(LIBAR): $(OBJ)
	@echo "    AR    $@"
	@$(AR) rcs -o $@ $^
	
%.o: %.c
	@echo "    CC    $@"
	@$(CC) -o $@ -c $< $(CFLAGS)

install:
	cp $(LIBAR) $(DESTDIR)/usr/lib/
	cp ansi.h $(DESTDIR)/usr/include/

clean:
	rm -f $(OBJ) $(LIBSO) $(LIBAR)
