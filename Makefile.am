AUTOMAKE_OPTIONS = gnu no-dependencies subdir-objects foreign
AM_CXXFLAGS = -I$(top_srcdir)/src

SUBDIRS = src data
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = milkcat

milkcat_SOURCES = src/main.cc
milkcat_LDADD = src/libmilkcat.la
