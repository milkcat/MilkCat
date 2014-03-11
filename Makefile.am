AUTOMAKE_OPTIONS = gnu no-dependencies subdir-objects foreign

AM_CXXFLAGS = -I$(top_srcdir)/src -DMODEL_PATH=\"$(pkgdatadir)/\" -std=c++11 -fno-rtti \
              -fno-exceptions -Wall -Werror

SUBDIRS = src data
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = milkcat neko mctools

milkcat_SOURCES = src/main.c
milkcat_LDADD = src/libmilkcat.la

neko_SOURCES = src/neko_main.cc
neko_LDADD = src/libmilkcat.la

mctools_SOURCES = src/mctools.cc 
mctools_LDADD = src/libmilkcat.la
