INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libshellscript.la

libshellscript_la_SOURCES = shellscript.cpp
libshellscript_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libshellscript_la_LIBADD = $(top_builddir)/kparts/libkparts.la


METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = shellscript.desktop
