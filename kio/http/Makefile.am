# $Id$
# Makefile.am of kdebase/kioslave/http

SUBDIRS = kcookiejar

INCLUDES=	$(all_includes) $(SSL_INCLUDES)
LDFLAGS =       $(all_libraries) $(SSL_LDFLAGS)

####### Files

bin_PROGRAMS = kio_http_cache_cleaner
lib_LTLIBRARIES = kio_http.la libkio_http_cache_cleaner.la

kio_http_la_SOURCES = http.cc extern_md5.c
kio_http_la_LIBADD = $(LIB_KIO) $(LIBZ) $(LIBSSL)
kio_http_la_LDFLAGS = -module -avoid-version

check_PROGRAMS = linktest
linktest_SOURCES = $(kio_http_la_SOURCES) linktest.cpp
linktest.cpp:
	echo "int main() {};" > linktest.cpp
linktest_LDFLAGS  = $(all_libraries) -module -avoid-version
linktest_LDADD = $(kio_http_la_LIBADD)

kio_http_cache_cleaner_SOURCES = http_cache_cleaner.cpp
kio_http_cache_cleaner_LDADD   = $(LIB_KDECORE)
kio_http_cache_cleaner_LDFLAGS = $(KDE_RPATH)

libkio_http_cache_cleaner_la_SOURCES = http_cache_cleaner.cpp
libkio_http_cache_cleaner_la_LIBADD  = $(LIB_KDECORE)
libkio_http_cache_cleaner_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = http.h extern_md5.h

kioconfig_DATA = http.desktop https.desktop
kioconfigdir = $(kde_confdir)/protocols

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = http_cache_cleaner.desktop

