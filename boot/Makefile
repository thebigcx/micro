.PHONY: clean all

all:
	@$(MAKE) -s -C stage1
	@echo "building boot image..."
	@tools/mkimage stage1/bootsect.bin

clean:
	@$(MAKE) -s -C stage1 clean
	@echo "RM    boot.img"
	@rm -f boot.img 