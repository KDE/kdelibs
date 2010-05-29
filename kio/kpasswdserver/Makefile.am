# $Id: Makefile.am 433401 2005-07-10 18:42:04Z adridg $
# Makefile.am of kdebase/kioslave/http

INCLUDES= -I$(top_srcdir)/kwallet/client $(all_includes)

####### Files

kde_module_LTLIBRARIES = kded_kpasswdserver.la

kded_kpasswdserver_la_SOURCES = kpasswdserver.cpp kpasswdserver.skel 
kded_kpasswdserver_la_METASOURCES = AUTO
kded_kpasswdserver_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_kpasswdserver_la_LIBADD = $(top_builddir)/kwallet/client/libkwalletclient.la $(LIB_KIO) $(LIB_KDED)

include $(top_srcdir)/admin/Doxyfile.am
