INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourceimap.h

kde_module_LTLIBRARIES = kabc_imap.la

kabc_imap_la_SOURCES = resourceimap.cpp

kabc_imap_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_imap_la_LIBADD	= ../../../kabc/libkabc.la ../../../kdeui/libkdeui.la
kabc_imap_la_COMPILE_FIRST = ../../kabc/addressee.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_imap.pot

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = imap.desktop
