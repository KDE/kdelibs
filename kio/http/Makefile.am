# $Id$
# Makefile.am of kdebase/kioslave/http

SUBDIRS = kcookiejar

INCLUDES=	-I$(srcdir)/.. -I$(srcdir)/../.. $(SSL_INCLUDES) $(all_includes)
LDFLAGS =       $(SSL_LDFLAGS) $(all_libraries)

####### Files

bin_PROGRAMS = kio_http_cache_cleaner
lib_LTLIBRARIES = kio_http.la kio_https.la kio_http_cache_cleaner.la

kio_https_la_SOURCES = https.cc extern_md5.c base64.c
kio_https_la_LIBADD = ../libkio.la $(LIB_KSSL) $(LIBZ)
kio_https_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined

kio_http_la_SOURCES = http.cc base64.c extern_md5.c
kio_http_la_LIBADD = ../libkio.la $(LIBZ)
kio_http_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined

kio_http_cache_cleaner_SOURCES = dummy.cpp
kio_http_cache_cleaner_LDADD   = kio_http_cache_cleaner.la
kio_http_cache_cleaner_LDFLAGS = $(KDE_RPATH)

kio_http_cache_cleaner_la_SOURCES = http_cache_cleaner.cpp
kio_http_cache_cleaner_la_LIBADD  = $(LIB_KIO)
kio_http_cache_cleaner_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = http.h extern_md5.h base64.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = http_cache_cleaner.desktop http.protocol https.protocol

dummy.cpp:
	echo > dummy.cpp

https.cc:	$(srcdir)/http.cc
	echo "#define HTTPS" > https.cc
	cat $(srcdir)/http.cc >> https.cc
