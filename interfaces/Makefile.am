# $Id$

INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libktexteditor.la

libktexteditor_la_SOURCES = ktexteditor.cpp
libktexteditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libktexteditor_la_LIBADD = $(top_builddir)/kparts/libkparts.la

include_HEADERS = ktexteditor.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = ktexteditor.desktop
