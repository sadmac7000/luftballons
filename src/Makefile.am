GENERIC_CFLAGS = -g -Wall -Wextra -DGL_GLEXT_PROTOTYPES -I../include
AM_CFLAGS = $(GENERIC_CFLAGS) --std=gnu99
AM_CXXFLAGS = $(GENERIC_CFLAGS) -fpermissive

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = luftcore.pc

lib_LTLIBRARIES = libluftcore.la

if BUILD_DEMO
noinst_PROGRAMS = demo

demo_SOURCES = demo.c
demo_LDADD = libluftcore.la
endif

libluftcore_la_SOURCES = \
	shader.c	\
	mesh.c		\
	vbuf.c		\
	ebuf.c		\
	interval.c	\
	matrix.c	\
	object.c	\
	vbuf_fmt.c	\
	bufpool.c	\
	draw_op.c	\
	uniform.c	\
	texmap.c	\
	quat.c		\
	refcount.c	\
	colorbuf.c	\
	draw_proc.c	\
	material.c	\
	state.c

if HAVE_COLLADA
libluftcore_la_SOURCES += dae_load.cc
endif

if HAVE_PNG
libluftcore_la_SOURCES += texmap_png.c
endif

if HAVE_TIFF
libluftcore_la_SOURCES += texmap_tiff.c
endif

LIB_CFLAGS = \
	$(collada_dom_CFLAGS)	\
	$(libpng_CFLAGS)	\
	$(OpenGL_CFLAGS)	\
	$(glut_CFLAGS)		\
	$(libtiff_CFLAGS)

libluftcore_la_CFLAGS = $(AM_CFLAGS) $(LIB_CFLAGS)
libluftcore_la_CXXFLAGS = $(AM_CXXFLAGS) $(LIB_CFLAGS)
libluftcore_la_LIBADD = 	\
	$(glut_LIBS)		\
	$(libtiff_LIBS)		\
	$(collada_dom_LIBS)	\
	$(OpenGL_LIBS)		\
	$(libpng_LIBS)
