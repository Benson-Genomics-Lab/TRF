AC_INIT([Tandem Repeats Finder], [4.10.0-rc.2], [https://github.com/Benson-Genomics-Lab/TRF/issues], [trf], [https://tandem.bu.edu/trf/trf.html])
dnl AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

AC_CHECK_HEADERS([limits.h stddef.h stdlib.h string.h])

AC_TYPE_SIZE_T

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_SEARCH_LIBS([floor], [m], [], [
  AC_MSG_ERROR([unable to find the floor() function])
])
AC_CHECK_FUNCS([sqrt memset strchr strerror strstr strtol])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
