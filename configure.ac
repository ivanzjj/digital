#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define([bubi_major_version],[1])
m4_define([bubi_minor_version],[0])
m4_define([bubi_micro_version],[0])
m4_define([bubi_version],[bubi_major_version.bubi_minor_version.bubi_micro_version])

AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIR([m4])
AC_INIT([bubi_project], [bubi_version], [zhangjianjun@bubi.cn])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
LT_INIT

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		src/Makefile
		src/database/Makefile
		src/ledger/Makefile
		src/base/Makefile
		Test/Makefile
		])

AC_OUTPUT
