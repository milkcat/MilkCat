#-*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(MilkCat, 0.3, ling032x@gmail.com)
AC_CONFIG_AUX_DIR([scripts])

AC_PROG_CC
AC_PROG_CXX
AM_INIT_AUTOMAKE([-Wall -Werror foreign]) 
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([tr1/unordered_map unordered_map])
AC_LANG_POP()

# Set the model dir
pkgdatadir=${datadir}/${PACKAGE}

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
