INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir)/interfaces -I$(srcdir) -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libshellscript.la

libshellscript_la_SOURCES = shellscript.cpp
libshellscript_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libshellscript_la_LIBADD = ../libkscript.la


METASOURCES = AUTO

servicetypedir = $(kde_servicesdir)
servicetype_DATA = shellscript.desktop
