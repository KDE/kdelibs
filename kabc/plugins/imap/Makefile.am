INCLUDES = -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourceimap.h

kde_module_LTLIBRARIES = kabc_imap.la

kabc_imap_la_SOURCES = resourceimap.cpp

kabc_imap_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_imap_la_LIBADD	= ../../../kabc/libkabc.la ../../../kdeui/libkdeui.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_imap.pot

linkdir		= $(kde_datadir)/kresources/contact
link_DATA	= imap.desktop
EXTRA_DIST	= $(link_DATA)
