# $Id$

INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libktexteditor.la libkregexpeditor.la

libktexteditor_la_SOURCES = ktexteditor.cpp
libktexteditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libktexteditor_la_LIBADD = $(top_builddir)/kparts/libkparts.la

libkregexpeditor_la_SOURCES = kregexpeditor.cpp
libkregexpeditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkregexpeditor_la_LIBADD =  $(LIB_KFILE) 

include_HEADERS = ktexteditor.h kregexpeditor.h kregexpeditorfactory.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = ktexteditor.desktop kregexpeditor.desktop
