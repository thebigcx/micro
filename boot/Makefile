.PHONY: clean all

all:
	@$(MAKE) -s -C stage1
	@$(MAKE) -s -C stage2
	@echo "building boot image..."
	@tools/mkimage stage1/bootsect.bin

clean:
	@$(MAKE) -s -C stage1 clean
	@$(MAKE) -s -C stage2 clean
	@echo "    RM boot.img"
	@rm -f boot.img 