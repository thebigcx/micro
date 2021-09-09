RULES := all clean install

$(RULES): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(RULES) $(SUBDIRS)
