AC_INIT([luftballons], [0.0], [devel@lists.luftengine.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign parallel-tests])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CXX
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

## Check for OpenGL ##
PKG_CHECK_MODULES([OpenGL], [gl])

## Check for GLUT ##
AC_ARG_WITH([glut], [AS_HELP_STRING(
	     [--with-glut],
	 [Allow usage of GLUT for demo apps])
], [], [with_glut=check])

glut_LIBS=
glut_CFLAGS=

AS_IF([test "x$with_glut" != xno], [
	AC_CHECK_LIB([glut], [glutInit], [
		AC_DEFINE([HAVE_GLUT], [1], [GLUT support enabled])
		with_glut=yes
		glut_LIBS=-lglut
	], [
		if test "x$with_glut" != xcheck; then
			AC_MSG_FAILURE([--with-glut requires GLUT-DOM])
		fi
	])
])

AC_ARG_VAR([glut_CFLAGS], [C compiler flags for GLUT])
AC_ARG_VAR([glut_LIBS], [linker flags for GLUT])

AM_CONDITIONAL([HAVE_GLUT], [ test "x$with_glut" = xyes ])

## Check for COLLADA ##
AC_ARG_WITH([collada], [AS_HELP_STRING(
	     [--with-collada],
	 [Support the COLLADA 1.4 model file format @<:@default=check@:>@])
], [], [with_collada=check])

AS_IF([test "x$with_collada" != xno], [
	PKG_CHECK_MODULES([collada_dom], [collada14dom], [
		AC_DEFINE([HAVE_COLLADA], [1], [COLLADA support enabled])
		with_collada=yes
	], [
		if test "x$with_collada" != xcheck; then
			AC_MSG_FAILURE([--with-collada requires COLLADA-DOM])
		fi
	])
])

AM_CONDITIONAL([HAVE_COLLADA], [ test "x$with_collada" = xyes ])

## Check for libpng ##
AC_ARG_WITH([png], [AS_HELP_STRING(
	     [--with-png],
	 [Support for importing PNG textures])
], [], [with_png=check])

AS_IF([test "x$with_png" != xno], [
	PKG_CHECK_MODULES([libpng], [libpng], [
		AC_DEFINE([HAVE_PNG], [1], [PNG support enabled])
		with_png=yes
	], [
		if test "x$with_png" != xcheck; then
			AC_MSG_FAILURE([--with-png requires libpng])
		fi
	])
])

AM_CONDITIONAL([HAVE_PNG], [ test "x$with_png" = xyes ])

## Check for libtiff ##
AC_ARG_WITH([tiff], [AS_HELP_STRING(
	     [--with-tiff],
	 [Support for importing TIFF textures])
], [], [with_tiff=check])

AS_IF([test "x$with_tiff" != xno], [
	PKG_CHECK_MODULES([libtiff], [libtiff-4], [
		AC_DEFINE([HAVE_TIFF], [1], [TIFF support enabled])
		with_tiff=yes
	], [
		if test "x$with_tiff" != xcheck; then
			AC_MSG_FAILURE([--with-tiff requires libtiff])
		fi
	])
])

AM_CONDITIONAL([HAVE_TIFF], [ test "x$with_tiff" = xyes ])

## Should we build the demo ##
AM_CONDITIONAL([BUILD_DEMO], [
	test "x$with_collada" = xyes -a	\
		"x$with_glut" = xyes -a	\
		"x$with_tiff" = xyes	\
])

LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/luftdraw.pc
	include/Makefile
])
AC_OUTPUT
