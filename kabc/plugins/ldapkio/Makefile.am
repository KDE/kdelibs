INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

# these are the headers for your project
noinst_HEADERS = resourceldapkioconfig.h

lib_LTLIBRARIES = libkabc_ldapkio.la
libkabc_ldapkio_la_SOURCES = resourceldapkio.cpp resourceldapkioconfig.cpp
libkabc_ldapkio_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libkabc_ldapkio_la_LIBADD	= $(top_builddir)/kabc/libkabc.la $(LIB_KIO)
libkabc_ldapkio_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h

kde_module_LTLIBRARIES = kabc_ldapkio.la
kabc_ldapkio_la_SOURCES = resourceldapkioplugin.cpp
kabc_ldapkio_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_ldapkio_la_LIBADD	= libkabc_ldapkio.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_ldapkio.pot

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = resourceldapkio.h

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = ldapkio.desktop

resourceldapkioplugin.lo: ../../addressee.h
