SUBDIRS = sample

INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkscriptloader.la 

libkscriptloader_la_SOURCES = scriptloader.cpp
libkscriptloader_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkscriptloader_la_LIBADD = $(top_builddir)/kparts/libkparts.la

include_HEADERS = scriptloader.h scriptinterface.h

METASOURCES = AUTO

#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = scriptinterface.desktop
