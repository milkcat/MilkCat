AUTOMAKE_OPTIONS = gnu no-dependencies subdir-objects foreign

SUBDIRS = src data
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = milkcat nekoneko mctools milkcat-keyphrase

milkcat_SOURCES = src/main.c
milkcat_LDADD = src/libmilkcat.la

nekoneko_SOURCES = src/nekoneko.c
nekoneko_LDADD = src/libmilkcat.la

milkcat_keyphrase_SOURCES = src/keyphrase.c
milkcat_keyphrase_LDADD = src/libmilkcat.la

mctools_SOURCES = src/mctools.cc
mctools_CXXFLAGS = -I$(top_srcdir)/src -fno-rtti -fno-exceptions
mctools_LDADD = src/libmilkcat.la
