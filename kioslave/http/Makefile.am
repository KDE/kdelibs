# $Id$
# Makefile.am of kdebase/kioslave/http

SUBDIRS = kcookiejar

INCLUDES= -I$(top_srcdir)/interfaces $(all_includes)
LDFLAGS = $(all_libraries)

####### Files

bin_PROGRAMS = kio_http_cache_cleaner
lib_LTLIBRARIES = kio_http_cache_cleaner.la
kde_module_LTLIBRARIES = kio_http.la

kio_http_la_SOURCES = http.cc httpfilter.cc
kio_http_la_METASOURCES = AUTO
kio_http_la_LIBADD = $(LIB_KIO) $(LIBZ)
kio_http_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

kio_http_cache_cleaner_SOURCES = dummy.cpp
kio_http_cache_cleaner_LDADD   = kio_http_cache_cleaner.la
kio_http_cache_cleaner_LDFLAGS = $(KDE_RPATH)

kio_http_cache_cleaner_la_SOURCES = http_cache_cleaner.cpp
kio_http_cache_cleaner_la_LIBADD  = $(LIB_KIO)
kio_http_cache_cleaner_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = http.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = http_cache_cleaner.desktop http.protocol https.protocol \
              webdav.protocol webdavs.protocol

dummy.cpp:
	echo > dummy.cpp
