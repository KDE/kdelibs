# $Id$

SUBDIRS = ktexteditor

INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkregexpeditor.la

libkregexpeditor_la_SOURCES = kregexpeditor.cpp 
libkregexpeditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkregexpeditor_la_LIBADD =  $(LIB_KFILE)

include_HEADERS = kregexpeditor.h kregexpdialoginterface.h

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kregexpeditor.desktop
