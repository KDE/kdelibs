INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcefileconfig.h

kde_module_LTLIBRARIES = kabc_file.la

kabc_file_la_SOURCES = resourcefile.cpp resourcefileconfig.cpp

kabc_file_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_file_la_LIBADD	= ../../libkabc.la ../../../kdeui/libkdeui.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_file.pot

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = resourcefile.h

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = file.desktop
