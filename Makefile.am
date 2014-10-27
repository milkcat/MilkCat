AUTOMAKE_OPTIONS = gnu no-dependencies subdir-objects foreign
AM_CXXFLAGS = -I$(top_srcdir)/src

SUBDIRS = src data
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = milkcat mctools

milkcat_SOURCES = src/main.cc
milkcat_LDADD = src/libmilkcat.la

mctools_SOURCES = src/mctools.cc
mctools_CXXFLAGS = -I$(top_srcdir)/src -fno-rtti -fno-exceptions
mctools_LDADD = src/libmilkcat.la
