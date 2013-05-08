TEST_PROGS= 

LTPROGS=demo

LIBS=luftdraw

demo_OBJS = demo.o

demo_DEPS = luftdraw

demo_LINK=-lglut -lGL -lm -lstdc++ \
	  -lcollada14dom -lboost_system -lpng -ltiff \
	  -lluftdraw

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

luftdraw_SONAME=0:0:0

include ../Makefile.defaults
