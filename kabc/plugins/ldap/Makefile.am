INCLUDES = $(all_includes) -I$(top_srcdir)

# these are the headers for your project
noinst_HEADERS   = resourceldap.h resourceldapconfig.h

kde_module_LTLIBRARIES = kabc_ldap.la

kabc_ldap_la_SOURCES = resourceldap.cpp resourceldapconfig.cpp

kabc_ldap_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN) $(LDAP_LIBS)
kabc_ldap_la_LIBADD	= ../../libkabc.la ../../../kdeui/libkdeui.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) resourceldap.cpp resourceldapconfig.cpp -o $(podir)/kabc_ldap.pot

linkdir		= $(kde_datadir)/kabc/
link_DATA	= ldap.plugin
EXTRA_DIST	= $(link_DATA)
