# $Id$

AM_CPPFLAGS = -DQT_NO_CAST_ASCII -UQT_NO_TRANSLATION
INCLUDES = -I$(top_srcdir)/kio/ksasl -I$(top_srcdir)/kio $(all_includes)

lib_LTLIBRARIES = ksasl_auth_digest.la
ksasl_auth_digest_la_LDFLAGS = -module $(KDE_PLUGIN)
ksasl_auth_digest_la_LIBADD = $(LIB_KSYCOCA)
ksasl_auth_digest_la_SOURCES = auth_digest.cpp

noinst_HEADERS = auth_digest.h

service_DATA = kauthdigestmodule.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO
