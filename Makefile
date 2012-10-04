.DEFAULT_GOAL := all

CLANGFLAGS = --std=gnu99
CFLAGS = -g -Wall -Wextra -DGL_GLEXT_PROTOTYPES -I/usr/include/collada-dom -I/usr/include/collada-dom/1.5
CC = gcc
CXX = g++
LINK = $(CC)
RM = rm
ifneq ($(QUIET),0)
override QUIET = @echo '   ' $1 $(if $2,$2,$@);$($1)
else
override QUIET = $($1)
endif

ifdef GCOV
GCOV_FLAGS = -fprofile-arcs -ftest-coverage
endif

TEST_PROGS= 

PROGS=luftballons

luftballons_OBJS =	\
	main.o		\
	shader.o	\
	mesh.o		\
	vbuf.o		\
	ebuf.o		\
	interval.o	\
	camera.o	\
	matrix.o	\
	object.o	\
	dae_load.o	\
	quat.o

luftballons_LINK=-lglut -lGL -lm -lstdc++ -lcollada15dom -lboost_system

define SET_DEPS =
$(1): $$($(1)_OBJS)
endef

$(foreach prog,$(TEST_PROGS),$(eval $(call SET_DEPS,$(prog))))
$(foreach prog,$(PROGS),$(eval $(call SET_DEPS,$(prog))))
$(foreach file,$(wildcard *.c),$(eval $(shell $(CC) -MM $(file) | sed 's/\\//')))
$(foreach file,$(wildcard *.cc),$(eval $(shell $(CXX) -MM $(file) | sed 's/\\//')))

$(TEST_PROGS):
	$(call QUIET,LINK) $(CFLAGS) $($@_LINK) $(GCOV_FLAGS) -o $@ $^

$(PROGS):
	$(call QUIET,LINK) $(CFLAGS) $($@_LINK) $(GCOV_FLAGS) -o $@ $^

.PHONY: check
check: $(TEST_PROGS)
	@echo
	@echo "Running checks"
	@echo
	@$(foreach prog,$^,./$(prog) && echo &&) echo "All checks completed successfully"

%.o: %.c
	$(call QUIET,CC) $(CLANGFLAGS) $(CFLAGS) $(GCOV_FLAGS) -c -o $@ $<

%.o: %.cc
	$(call QUIET,CXX) $(CFLAGS) $(GCOV_FLAGS) -fpermissive -c -o $@ $<

.PHONY: clean
clean:
	$(call QUIET,RM,*.o) -f *.o
	$(call QUIET,RM,*.gcda) -f *.gcda
	$(call QUIET,RM,*.gcno) -f *.gcno
	$(call QUIET,RM,*.gcov) -f *.gcov
	$(call QUIET,RM,$(TEST_PROGS)) -f $(TEST_PROGS)
	$(call QUIET,RM,$(PROGS)) -f $(PROGS)

.PHONY: all
all: $(PROGS)
