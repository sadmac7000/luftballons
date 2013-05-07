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

luftdraw_SONAME=0:0:0

include ../Makefile.defaults
