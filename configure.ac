AC_INIT([luftballons], [0.0], [casey.dahlin@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign parallel-tests])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CXX
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	include/Makefile
])
AC_OUTPUT
