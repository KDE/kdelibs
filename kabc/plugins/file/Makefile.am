INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcefileconfig.h

lib_LTLIBRARIES = libkabc_file.la
libkabc_file_la_SOURCES = resourcefile.cpp resourcefileconfig.cpp
libkabc_file_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
libkabc_file_la_LIBADD = $(LIB_KABC)
libkabc_file_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h


kde_module_LTLIBRARIES = kabc_file.la
kabc_file_la_SOURCES = resourcefileplugin.cpp
kabc_file_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabc_file_la_LIBADD = libkabc_file.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_file.pot

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = resourcefile.h

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = file.desktop
