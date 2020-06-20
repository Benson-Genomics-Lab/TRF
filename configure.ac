AC_INIT([Tandem Repeats Finder], [4.10.0-rc.2], [https://github.com/Benson-Genomics-Lab/TRF/issues], [trf], [https://tandem.bu.edu/trf/trf.html])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
dnl AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LN_S

AC_CHECK_HEADERS([limits.h stddef.h stdlib.h string.h])

AC_TYPE_SIZE_T
TARGET_NAME=""
AS_CASE([$target_cpu],
  [x86_64],
  [
    TARGET_NAME="64"
  ],
  [i?68],
  [
    TARGET_NAME="32"
  ]
)
AS_CASE([$target_os],
  [linux*],
  [
    TARGET_NAME="linux$TARGET_NAME"
  ],
  [darwin*],
  [
    TARGET_NAME="mac"
  ],
  [cygwin*|mingw*],
  [
    TARGET_NAME="dos$TARGET_NAME"
  ]
)
AC_SUBST([TARGET_NAME])

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_SEARCH_LIBS([floor], [m], [], [
  AC_MSG_ERROR([unable to find the floor() function])
])
AC_CHECK_FUNCS([sqrt memset strchr strerror strstr strtol])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
