INCLUDES = -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcefile.h resourcefileconfig.h

kde_module_LTLIBRARIES = kabc_file.la

kabc_file_la_SOURCES = resourcefile.cpp resourcefileconfig.cpp

kabc_file_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN) $(LDAP_LIBS)
kabc_file_la_LIBADD	= $(top_srcdir)/kresources/libkderesources.la ../../libkabc.la \
  ../../../kdeui/libkdeui.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_file.pot

linkdir		= $(kde_datadir)/kresources/contact
link_DATA	= file.desktop
EXTRA_DIST	= $(link_DATA)
