# $Id$
# Makefile.am of kdebase/kioslave/http

INCLUDES=	$(all_includes)
LDFLAGS =       $(all_libraries) $(KDE_RPATH)
LDADD	=	$(LIB_KIO) $(LIBZ) $(LIBSSL)

####### Files

bin_PROGRAMS = kio_http

kio_http_SOURCES = http.cc extern_md5.c
noinst_HEADERS = http.h extern_md5.h

kioconfig_DATA = http.kdelnk https.kdelnk
kioconfigdir = $(kde_confdir)/protocols

messages:

