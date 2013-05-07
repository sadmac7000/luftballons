.DEFAULT_GOAL := all

CLANGFLAGS = --std=gnu99
CFLAGS = -g -Wall -Wextra -DGL_GLEXT_PROTOTYPES -isystem/usr/include/collada-dom -isystem/usr/include/collada-dom/1.4
CC = gcc
CXX = g++
LIBTOOL = libtool --silent
LTCC = $(LIBTOOL) --mode=compile $(CC)
LTCXX = $(LIBTOOL) --mode=compile $(CXX)
LINK = $(CC)
LTLINK = $(LIBTOOL) --mode=link $(LINK)
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

LIBS=luftdraw

luftballons_OBJS =	\
	main.o		\
	shader.o	\
	mesh.o		\
	vbuf.o		\
	ebuf.o		\
	interval.o	\
	matrix.o	\
	object.o	\
	dae_load.o	\
	vbuf_fmt.o	\
	bufpool.o	\
	draw_queue.o	\
	uniform.o	\
	texmap.o	\
	texmap_png.o	\
	texmap_tiff.o	\
	quat.o		\
	refcount.o	\
	colorbuf.o	\
	target.o	\
	state.o

luftballons_LINK=-lglut -lGL -lm -lstdc++ \
		 -lcollada14dom -lboost_system -lpng -ltiff

luftdraw_OBJS =		\
	shader.lo	\
	mesh.lo		\
	vbuf.lo		\
	ebuf.lo		\
	interval.lo	\
	matrix.lo	\
	object.lo	\
	dae_load.lo	\
	vbuf_fmt.lo	\
	bufpool.lo	\
	draw_queue.lo	\
	uniform.lo	\
	texmap.lo	\
	texmap_png.lo	\
	texmap_tiff.lo	\
	quat.lo		\
	refcount.lo	\
	colorbuf.lo	\
	target.lo	\
	state.lo

luftdraw_LINK=-lglut -lGL -lm -lstdc++ \
	      -lcollada14dom -lboost_system -lpng -ltiff

define SET_DEPS =
$(1): $$($(1)_OBJS)
endef

$(foreach prog,$(TEST_PROGS),$(eval $(call SET_DEPS,$(prog))))
$(foreach prog,$(PROGS),$(eval $(call SET_DEPS,$(prog))))
$(foreach lib,$(LIBS),$(eval $(call SET_DEPS,$(lib))))
$(foreach file,$(wildcard *.c),$(eval $(shell $(CC) -MM $(file) | sed 's/\\//')))
$(foreach file,$(wildcard *.h),$(eval $(shell $(CC) -MM $(file) | sed 's/\\//')))
$(foreach file,$(wildcard *.cc),$(eval $(shell $(CXX) -MM $(file) | sed 's/\\//')))

$(TEST_PROGS):
	$(call QUIET,LINK) $(CFLAGS) $($@_LINK) $(GCOV_FLAGS) -o $@ $^

$(PROGS):
	$(call QUIET,LINK) $(CFLAGS) $($@_LINK) $(GCOV_FLAGS) -o $@ $^

.PHONY: $(LIBS)
$(LIBS):
	$(call QUIET,LTLINK) $(CFLAGS) $($@_LINK) $(GCOV_FLAGS) O -o lib$@.la $^

lib%.la: %

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

%.lo %.o: %.c
	$(call QUIET,LTCC) $(CLANGFLAGS) $(CFLAGS) $(GCOV_FLAGS) -c -o $@ $<

%.lo %.o: %.cc
	$(call QUIET,LTCXX) $(CFLAGS) $(GCOV_FLAGS) -fpermissive -c -o $@ $<

.PHONY: clean
clean:
	$(call QUIET,RM,\*.o) -f *.o
	$(call QUIET,RM,\*.lo) -f *.lo
	$(call QUIET,RM,\*.la) -f *.la
	$(call QUIET,RM,\*.gcda) -f *.gcda
	$(call QUIET,RM,\*.gcno) -f *.gcno
	$(call QUIET,RM,\*.gcov) -f *.gcov
	$(call QUIET,RM,$(TEST_PROGS)) -f $(TEST_PROGS)
	$(call QUIET,RM,$(PROGS)) -f $(PROGS)

.PHONY: all
all: $(PROGS) $(LIBS)
