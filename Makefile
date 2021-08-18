all:
	@echo "building kernel..."
	@$(MAKE) -s -C kernel
	@echo "building bootloader..."
	@$(MAKE) -s -C boot

clean:
	@$(MAKE) -s -C kernel clean
	@$(MAKE) -s -C boot clean
