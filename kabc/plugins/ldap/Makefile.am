INCLUDES = $(LDAP_INCS) -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS = resourceldapconfig.h

lib_LTLIBRARIES = libkabc_ldap.la
libkabc_ldap_la_SOURCES = resourceldap.cpp resourceldapconfig.cpp
libkabc_ldap_la_LDFLAGS	= $(KDE_RPATH) $(LDAP_LIBS) $(all_libraries) -version-info 1:0:0 -no-undefined
libkabc_ldap_la_LIBADD	= $(LIB_KABC)
libkabc_ldap_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h

kde_module_LTLIBRARIES = kabc_ldap.la
kabc_ldap_la_SOURCES = resourceldapplugin.cpp
kabc_ldap_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -no-undefined
kabc_ldap_la_LIBADD	= libkabc_ldap.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_ldap.pot

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = resourceldap.h

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = ldap.desktop
