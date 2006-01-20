INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_builddir) $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

# these are the headers for your project
noinst_HEADERS = resourcedirconfig.h

lib_LTLIBRARIES = libkabc_dir.la
libkabc_dir_la_SOURCES = resourcedir.cpp resourcedirconfig.cpp
libkabc_dir_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined 
libkabc_dir_la_LIBADD = $(LIB_KABC)
libkabc_dir_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h

kde_module_LTLIBRARIES = kabc_dir.la
kabc_dir_la_SOURCES = resourcedirplugin.cpp
kabc_dir_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
kabc_dir_la_LIBADD = libkabc_dir.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_dir.pot

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = resourcedir.h

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = dir.desktop

resourcedirplugin.lo: ../../addressee.h
