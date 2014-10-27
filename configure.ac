#-*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(MilkCat, 0.3, ling032x@gmail.com)
AC_CONFIG_AUX_DIR([scripts])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

AM_INIT_AUTOMAKE([-Wall -Werror foreign]) 
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR
LT_INIT

#AX_PTHREAD

#LIBS="$PTHREAD_LIBS $LIBS"
#CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
#CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"

#AX_CXX_COMPILE_STDCXX_11

# Checks for header files.
AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([tr1/unordered_map unordered_map])
AC_LANG_POP()

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strlcpy])

# Set the model dir
pkgdatadir=${datadir}/${PACKAGE}

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile data/Makefile])
AC_OUTPUT
