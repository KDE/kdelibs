# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir) $(all_includes)

# these are the headers for your project
noinst_HEADERS = sdevent.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

lib_LTLIBRARIES =  libkdnssd.la

libkdnssd_la_SOURCES = remoteservice.cpp responder.cpp servicebase.cpp \
				settings.cpp publicservice.cpp query.cpp domainbrowser.cpp servicebrowser.cpp
dnssdincludedir = $(includedir)/dnssd
dnssdinclude_HEADERS = domainbrowser.h query.h remoteservice.h \
	publicservice.h servicebase.h servicebrowser.h settings.h
libkdnssd_la_LIBADD = ../kdecore/libkdecore.la $(LIB_DNSSD)
libkdnssd_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0

#kde_kcfg_DATA = kcm_kdnssd.kcfg

settings.cpp: settings.h
settings.h: kcm_kdnssd.kcfg settings.kcfgc $(top_srcdir)/kdecore/kconfig_compiler/kconfig_compiler
	$(top_srcdir)/kdecore/kconfig_compiler/kconfig_compiler kcm_kdnssd.kcfg settings.kcfgc; ret=$$?; \
	if test "$$ret" != 0; then rm -f settings.h ; exit $$ret ;  fi
