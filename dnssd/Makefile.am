# set the include path for X, qt and KDE
INCLUDES = -I.. $(all_includes)

# these are the headers for your project
noinst_HEADERS = sdevent.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

lib_LTLIBRARIES =  libkdnssd.la

libkdnssd_la_SOURCES = remoteservice.cpp responder.cpp servicebase.cpp \
				settings.kcfgc publicservice.cpp query.cpp domainbrowser.cpp servicebrowser.cpp
dnssdincludedir = $(includedir)/dnssd
dnssdinclude_HEADERS = domainbrowser.h query.h remoteservice.h \
	publicservice.h servicebase.h servicebrowser.h 
libkdnssd_la_LIBADD = $(LIB_DNSSD)

kde_kcfg_DATA = kcm_kdnssd.kcfg
AM_CXXFLAGS = -O0
