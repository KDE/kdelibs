# $Id$
# Makefile.am of kdebase/kioslave/http

INCLUDES=	$(all_includes)

####### Files

bin_PROGRAMS = kcookiejar
lib_LTLIBRARIES = kcookiejar.la
kde_module_LTLIBRARIES = kded_kcookiejar.la

kcookiejar_la_SOURCES = main.cpp
METASOURCES = AUTO
kcookiejar_la_LDFLAGS = $(all_libraries) -module -avoid-version
kcookiejar_la_LIBADD = $(LIB_KDECORE)

kded_kcookiejar_la_SOURCES = kcookiejar.cpp kcookieserver.cpp \
			kcookieserver.skel kcookiewin.cpp 
kded_kcookiejar_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_kcookiejar_la_LIBADD = $(LIB_KIO)

kcookiejar_SOURCES = dummy.cpp
kcookiejar_LDADD = kcookiejar.la
kcookiejar_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kded_DATA = kcookiejar.desktop
kdeddir = $(kde_servicesdir)/kded

update_DATA = kcookiescfg.upd
updatedir = $(kde_datadir)/kconf_update

dummy.cpp:
	echo > dummy.cpp
