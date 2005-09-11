# $Id$
# Makefile.am of kdebase/kioslave/http

SUBDIRS = kcookiejar

INCLUDES= -I$(top_srcdir)/interfaces -I$(top_srcdir)/kio/httpfilter -I$(top_srcdir)/kdecore/network $(all_includes) $(GSSAPI_INCS)
AM_LDFLAGS = $(all_libraries) $(GSSAPI_RPATH)

####### Files

bin_PROGRAMS=
lib_LTLIBRARIES=
kdeinit_LTLIBRARIES = kio_http_cache_cleaner.la
kde_module_LTLIBRARIES = kio_http.la

kio_http_la_SOURCES = http.cc
kio_http_la_METASOURCES = AUTO
kio_http_la_LIBADD = $(LIB_KIO) $(top_builddir)/kio/httpfilter/libhttpfilter.la $(top_builddir)/kio/misc/kntlm/libkntlm.la
kio_http_la_LDFLAGS = $(all_libraries) $(GSSAPI_RPATH) -module $(KDE_PLUGIN) $(GSSAPI_LIBS)

kio_http_cache_cleaner_la_SOURCES = http_cache_cleaner.cpp
kio_http_cache_cleaner_la_LIBADD  = $(LIB_KIO)
kio_http_cache_cleaner_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = http.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = http_cache_cleaner.desktop http.protocol https.protocol \
              webdav.protocol webdavs.protocol

include $(top_srcdir)/admin/Doxyfile.am
