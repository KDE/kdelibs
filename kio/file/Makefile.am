## Makefile.am of kdebase/kioslave/file

INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../.. $(all_includes)

####### Files

lib_LTLIBRARIES = kio_file.la

kio_file_la_SOURCES = file.cc
kio_file_la_LIBADD = ../libkio.la
kio_file_la_LDFLAGS = -module -avoid-version -no-undefined
noinst_HEADERS = file.h

kioconfig_DATA = file.desktop
kioconfigdir = $(kde_confdir)/protocols
