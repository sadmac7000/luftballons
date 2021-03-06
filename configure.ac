AC_INIT([luftballons], [0.0], [devel@lists.luftengine.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign parallel-tests])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CXX
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

## Check for OpenGL ##
PKG_CHECK_MODULES([OpenGL], [gl])

## Check for GLFW ##
AC_ARG_WITH([glfw], [AS_HELP_STRING(
	     [--with-glfw],
	 [Allow usage of GLFW for test apps])
], [], [with_glfw=yes])

glfw_LIBS=
glfw_CFLAGS=

AS_IF([test "x$with_glfw" != xno], [
	AC_CHECK_LIB([glfw], [glfwInit], [
		AC_DEFINE([HAVE_GLFW], [1], [GLFW support enabled])
		with_glfw=yes
		glfw_LIBS=-lglfw
	], [
		AC_MSG_FAILURE([--with-glfw requires GLFW])
	])
])

AC_ARG_VAR([glfw_CFLAGS], [C compiler flags for GLFW])
AC_ARG_VAR([glfw_LIBS], [linker flags for GLFW])

AM_CONDITIONAL([HAVE_GLFW], [ test "x$with_glfw" = xyes ])

## Check for COLLADA ##
AC_ARG_WITH([collada], [AS_HELP_STRING(
	     [--with-collada],
	 [Support the COLLADA 1.4 model file format @<:@default=yes@:>@])
], [], [with_collada=yes])

AS_IF([test "x$with_collada" != xno], [
	PKG_CHECK_MODULES([collada_dom], [collada-dom-141], [
		AC_DEFINE([HAVE_COLLADA], [1], [COLLADA support enabled])
		with_collada=yes
	], [
		AC_MSG_FAILURE([--with-collada requires COLLADA-DOM])
	])
])

AM_CONDITIONAL([HAVE_COLLADA], [ test "x$with_collada" = xyes ])

## Check for libpng ##
AC_ARG_WITH([png], [AS_HELP_STRING(
	     [--with-png],
	 [Support for importing PNG textures @<:@default=yes@:>@])
], [], [with_png=yes])

AS_IF([test "x$with_png" != xno], [
	PKG_CHECK_MODULES([libpng], [libpng], [
		AC_DEFINE([HAVE_PNG], [1], [PNG support enabled])
		with_png=yes
	], [
		AC_MSG_FAILURE([--with-png requires libpng])
	])
])

AM_CONDITIONAL([HAVE_PNG], [ test "x$with_png" = xyes ])

## Check for libtiff ##
AC_ARG_WITH([tiff], [AS_HELP_STRING(
	     [--with-tiff],
	 [Support for importing TIFF textures @<:@default=yes@:>@])
], [], [with_tiff=yes])

AS_IF([test "x$with_tiff" != xno], [
	PKG_CHECK_MODULES([libtiff], [libtiff-4], [
		AC_DEFINE([HAVE_TIFF], [1], [TIFF support enabled])
		with_tiff=yes
	], [
		AC_MSG_FAILURE([--with-tiff requires libtiff])
	])
])

AM_CONDITIONAL([HAVE_TIFF], [ test "x$with_tiff" = xyes ])

## Should we build the demo ##
AM_CONDITIONAL([BUILD_DEMO], [
	test "x$with_collada" = xyes -a	\
		"x$with_glfw" = xyes -a	\
		"x$with_tiff" = xyes	\
])

### Vector configuration ###

## Vector base size ##
AC_ARG_WITH([vec-base], [AS_HELP_STRING(
	     [--with-vec-base],
	 [Minimum space allocation for variable-sized arrays (2^x) @<:@default=3@:>@])
], [], [with_vec_base=3])

AS_IF([test $with_vec_base -ge 1], [], [
       AC_MSG_FAILURE([vec-base must be >= 1])
])

AC_DEFINE_UNQUOTED([VEC_BASE_SIZE], [(1 << $with_vec_base)],
	  [Minimum space allocation for variable-sized arrays])

## Vector stingy reclamation ##
AC_ARG_WITH([stingy-vecs], [AS_HELP_STRING(
	     [--with-stingy-vecs],
     [Be more aggressive about reclaiming space in variable-sized arrays @<:@default=no@:>@])
], [], [with_stingy_vecs=no])

AS_IF([test "x$with_stingy_vecs" != xno], [
		AC_DEFINE([VEC_STINGY], [1], [Stingy vectors])
], [])

## Misc. Init ##
LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/luftcore.pc
	include/Makefile
])
AC_OUTPUT
