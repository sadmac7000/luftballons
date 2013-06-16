GENERIC_CFLAGS = -g -Wall -Wextra -DGL_GLEXT_PROTOTYPES -I../include
AM_CFLAGS = $(GENERIC_CFLAGS) --std=gnu99
AM_CXXFLAGS = $(GENERIC_CFLAGS) -fpermissive

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = luftdraw.pc

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
	draw_op.c	\
	uniform.c	\
	texmap.c	\
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

if HAVE_TIFF
libluftdraw_la_SOURCES += texmap_tiff.c
endif

LIB_CFLAGS = \
	$(collada_dom_CFLAGS)	\
	$(libpng_CFLAGS)	\
	$(OpenGL_CFLAGS)	\
	$(glut_CFLAGS)		\
	$(libtiff_CFLAGS)

libluftdraw_la_CFLAGS = $(AM_CFLAGS) $(LIB_CFLAGS)
libluftdraw_la_CXXFLAGS = $(AM_CXXFLAGS) $(LIB_CFLAGS)
libluftdraw_la_LIBADD = 	\
	$(glut_LIBS)		\
	$(libtiff_LIBS)		\
	$(collada_dom_LIBS)	\
	$(OpenGL_LIBS)		\
	$(libpng_LIBS)
