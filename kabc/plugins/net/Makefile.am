INCLUDES = -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcenet.h resourcenetconfig.h

kde_module_LTLIBRARIES = kabc_net.la

kabc_net_la_SOURCES = resourcenet.cpp resourcenetconfig.cpp

kabc_net_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_net_la_LIBADD	= $(top_srcdir)/kabc/libkabc.la \
	$(top_srcdir)/kdeui/libkdeui.la \
	$(top_srcdir)/kio/libkio.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_net.pot

linkdir		= $(kde_datadir)/kabc/plugins
link_DATA	= net.desktop
EXTRA_DIST = $(link_DATA)
