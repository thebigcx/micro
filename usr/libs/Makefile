SUBDIRS = libc

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
	$(MAKE) -C $@ install

.PHONY: all $(SUBDIRS)