INCLUDES= -I$(srcdir)/../.. -I$(srcdir)/.. $(all_includes)

####### Files

lib_LTLIBRARIES = kio_ftp.la

kio_ftp_la_SOURCES = ftp.cc
kio_ftp_la_LIBADD  = ../libkio.la #-lkioslave
kio_ftp_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = ftp.h

kioconfig_DATA = ftp.desktop
kioconfigdir = $(kde_confdir)/protocols

check_PROGRAMS = linktest
linktest_SOURCES = $(kio_ftp_la_SOURCES) linktest.cpp
linktest.cpp:
	echo "int main() {};" > linktest.cpp
linktest_LDFLAGS  = $(all_libraries) -module -avoid-version
linktest_LDADD = $(kio_ftp_la_LIBADD)

