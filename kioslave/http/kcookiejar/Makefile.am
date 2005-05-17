# Makefile.am of kdebase/kioslave/http

SUBDIRS=tests
INCLUDES=	$(all_includes)

####### Files

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kcookiejar.la
kde_module_LTLIBRARIES = kded_kcookiejar.la

kcookiejar_la_SOURCES = main.cpp
METASOURCES = AUTO
kcookiejar_la_LDFLAGS = $(all_libraries) -module -avoid-version
kcookiejar_la_LIBADD = $(LIB_KDECORE)

kded_kcookiejar_la_SOURCES = kcookiejar.cpp kcookieserver.cpp \
			kcookieserver.skel kcookiewin.cpp 
kded_kcookiejar_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_kcookiejar_la_LIBADD = $(LIB_KIO) $(LIB_KDED)

kded_DATA = kcookiejar.desktop
kdeddir = $(kde_servicesdir)/kded

update_DATA = kcookiescfg.upd
updatedir = $(kde_datadir)/kconf_update

cookie_DATA = domain_info
cookiedir = $(kde_datadir)/khtml

