INCLUDES = -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcedir.h resourcedirconfig.h

kde_module_LTLIBRARIES = kabc_dir.la

kabc_dir_la_SOURCES = resourcedir.cpp resourcedirconfig.cpp

kabc_dir_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_dir_la_LIBADD	= ../../libkabc.la ../../../kdeui/libkdeui.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_dir.pot

linkdir		= $(kde_datadir)/kabc/plugins
link_DATA	= dir.desktop
EXTRA_DIST	= $(link_DATA)
