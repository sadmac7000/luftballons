TARGETS = clean all install
SUBDIRS = src include

.PHONY: $(TARGETS)

$(TARGETS):
	+$(foreach sdir, $(SUBDIRS), $(call make_subdir,$(sdir)))

define make_subdir = 
$(MAKE) -C $(1) $@

endef

.DEFAULT_GOAL := all
