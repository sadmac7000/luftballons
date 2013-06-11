AC_INIT([luftballons], [0.0], [devel@lists.luftengine.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign parallel-tests])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CXX
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

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

LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	include/Makefile
])
AC_OUTPUT
