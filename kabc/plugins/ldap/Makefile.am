INCLUDES = $(LDAP_INCS) -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourceldap.h resourceldapconfig.h

kde_module_LTLIBRARIES = kabc_ldap.la

kabc_ldap_la_SOURCES = resourceldap.cpp resourceldapconfig.cpp

kabc_ldap_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN) $(LDAP_LIBS)
kabc_ldap_la_LIBADD	= $(LIB_KABC)
kabc_ldap_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_ldap.pot

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = ldap.desktop
