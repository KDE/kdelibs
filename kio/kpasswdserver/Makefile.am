# $Id$
# Makefile.am of kdebase/kioslave/http

INCLUDES=	$(all_includes)

####### Files

lib_LTLIBRARIES = kded_kpasswdserver.la

kded_kpasswdserver_la_SOURCES = kpasswdserver.cpp kpasswdserver.skel 
kded_kpasswdserver_la_METASOURCES = AUTO
kded_kpasswdserver_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_kpasswdserver_la_LIBADD = $(LIB_KIO)
