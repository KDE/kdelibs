INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS = resourcenet.h resourcenetconfig.h

kde_module_LTLIBRARIES = kabc_net.la

kabc_net_la_SOURCES = resourcenet.cpp resourcenetconfig.cpp
kabc_net_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_net_la_LIBADD	= $(top_builddir)/kabc/libkabc.la $(LIB_KIO)
kabc_ldap_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_net.pot

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = net.desktop
