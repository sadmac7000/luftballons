GENERIC_CFLAGS = -g -Wall -Wextra -DGL_GLEXT_PROTOTYPES -I../include
AM_CFLAGS = $(GENERIC_CFLAGS) --std=gnu99
AM_CXXFLAGS = $(GENERIC_CFLAGS) -fpermissive

lib_LTLIBRARIES = libluftdraw.la

if BUILD_DEMO
noinst_PROGRAMS = demo

demo_SOURCES = demo.c
demo_LDADD = libluftdraw.la
endif

libluftdraw_la_SOURCES = \
	shader.c	\
	mesh.c		\
	vbuf.c		\
	ebuf.c		\
	interval.c	\
	matrix.c	\
	object.c	\
	vbuf_fmt.c	\
	bufpool.c	\
	draw_queue.c	\
	uniform.c	\
	texmap.c	\
	texmap_tiff.c	\
	quat.c		\
	refcount.c	\
	colorbuf.c	\
	target.c	\
	state.c

if HAVE_COLLADA
libluftdraw_la_SOURCES += dae_load.cc
endif

if HAVE_PNG
libluftdraw_la_SOURCES += texmap_png.c
endif

libluftdraw_la_CFLAGS = $(AM_CFLAGS) $(collada_dom_CFLAGS) $(libpng_CFLAGS)
libluftdraw_la_CXXFLAGS = $(AM_CXXFLAGS) $(collada_dom_CFLAGS) $(libpng_CFLAGS)
libluftdraw_la_LIBADD = -lglut -lGL -lm -lstdc++ \
	  -lboost_system -ltiff $(collada_dom_LIBS) $(libpng_LIBS)
