# $Id$
# Makefile.am of kdebase/kioslave/http

SUBDIRS = kcookiejar

INCLUDES=	$(all_includes) $(SSL_INCLUDES)
LDFLAGS =       $(all_libraries) $(SSL_LDFLAGS) $(KDE_RPATH)

####### Files

bin_PROGRAMS = kio_http kio_http_cache_cleaner
lib_LTLIBRARIES = libkio_http.la libkio_http_cache_cleaner.la

libkio_http_la_SOURCES = http.cc extern_md5.c
libkio_http_la_LIBADD	=	$(LIB_KIO) $(LIBZ) $(LIBSSL)
libkio_http_la_LDFLAGS	=	-module

kio_http_SOURCES = http.cc extern_md5.c
kio_http_LDADD	 = $(LIB_KIO) $(LIBZ) $(LIBSSL)

kio_http_cache_cleaner_SOURCES = http_cache_cleaner.cpp
kio_http_cache_cleaner_LDADD   = $(LIB_KDECORE)

libkio_http_cache_cleaner_la_SOURCES = http_cache_cleaner.cpp
libkio_http_cache_cleaner_la_LIBADD  = $(LIB_KDECORE)
libkio_http_cache_cleaner_la_LDFLAGS = -module

noinst_HEADERS = http.h extern_md5.h

kioconfig_DATA = http.desktop https.desktop
kioconfigdir = $(kde_confdir)/protocols

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = http_cache_cleaner.desktop

