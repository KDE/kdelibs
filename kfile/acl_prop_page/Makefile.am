# $Id$

# Automake sucks:
# automake: kfile/acl_prop_page/Makefile.am: warning: automake does not 
# support libkacl_prop_page_la_LDFLAGS being defined conditionally
# gmake[1]: *** [cvs] Error 1

LDFLAGS = -module $(KDE_PLUGIN)

lib_LTLIBRARIES = $(ACL_MODULE)

if USE_POSIX_ACL
ACL_MODULE = libkacl_prop_page.la
endif

libkacl_prop_page_la_LIBADD = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_QT) $(LIBPOSIX1E)
libkacl_prop_page_la_SOURCES = acl.cpp kaclmanipulation.cpp
libkacl_prop_page_la_METASOURCES = AUTO

noinst_HEADERS = acl.h kaclmanipulation.h

service_DATA = kaclplugin.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO
