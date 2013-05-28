GENERIC_CFLAGS = -g -Wall -Wextra -DGL_GLEXT_PROTOTYPES \
	 -isystem/usr/include/collada-dom \
	 -isystem/usr/include/collada-dom/1.4 -fvisibility=hidden \
	 -I../include -fvisibility=hidden
AM_CFLAGS = $(GENERIC_CFLAGS) --std=gnu99
AM_CXXFLAGS = $(GENERIC_CFLAGS) -fpermissive

lib_LTLIBRARIES = libluftdraw.la

noinst_PROGRAMS = demo

demo_SOURCES = demo.c
demo_LDADD = libluftdraw.la

libluftdraw_la_SOURCES = \
	shader.c	\
	mesh.c		\
	vbuf.c		\
	ebuf.c		\
	interval.c	\
	matrix.c	\
	object.c	\
	dae_load.cc	\
	vbuf_fmt.c	\
	bufpool.c	\
	draw_queue.c	\
	uniform.c	\
	texmap.c	\
	texmap_png.c	\
	texmap_tiff.c	\
	quat.c		\
	refcount.c	\
	colorbuf.c	\
	target.c	\
	state.c
libluftdraw_la_LIBADD = -lglut -lGL -lm -lstdc++ \
	  -lcollada14dom -lboost_system -lpng -ltiff
