## Makefile.am of kdebase/kioslave/file

INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../.. $(all_includes)

####### Files

lib_LTLIBRARIES = kio_file.la

kio_file_la_SOURCES = file.cc
kio_file_la_LIBADD = ../libkio.la
kio_file_la_LDFLAGS = $(all_libraries) -module -avoid-version
noinst_HEADERS = file.h

check_PROGRAMS = test
test_SOURCES = $(kio_file_la_SOURCES) test.cpp
test.cpp:
	echo "int main() {};" > test.cpp
test_LDFLAGS  = $(all_libraries) -module -avoid-version
test_LDADD = $(kio_file_la_LIBADD)

kioconfig_DATA = file.desktop
kioconfigdir = $(kde_confdir)/protocols
