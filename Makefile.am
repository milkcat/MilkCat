AUTOMAKE_OPTIONS = gnu no-dependencies subdir-objects foreign

SUBDIRS = src data
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = milkcat

milkcat_SOURCES = src/main.c
milkcat_LDADD = src/libmilkcat.la
