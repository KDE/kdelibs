# $Id$
# Makefile.am of kdebase/kioslave/http

INCLUDES= -I$(srcdir)/.. $(all_includes) 

####### Files

lib_LTLIBRARIES = libkded_test.la

libkded_test_la_SOURCES = test.cpp test.skel
libkded_test_la_METASOURCES = AUTO
libkded_test_la_LDFLAGS = $(all_libraries) -module -avoid-version
libkded_test_la_LIBADD = $(top_builddir)/kdecore/libkdecore.la

servicesdir = $(kde_servicesdir)/kded
services_DATA = test.desktop

