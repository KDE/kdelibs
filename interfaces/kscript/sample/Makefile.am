INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir)/interfaces -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/interfaces/kscript $(all_includes)

kde_module_LTLIBRARIES = libshellscript.la

libshellscript_la_SOURCES = shellscript.cpp
libshellscript_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -no-undefined -avoid-version
libshellscript_la_LIBADD = ../libkscript.la


METASOURCES = AUTO

servicetypedir = $(kde_servicesdir)
servicetype_DATA = shellscript.desktop
