# $Id$

INCLUDES = -I$(top_srcdir)/kio/ksasl -I$(top_srcdir)/kio $(all_includes)

lib_LTLIBRARIES = ksasl_auth_anon.la

ksasl_auth_anon_la_LDFLAGS = -module $(KDE_PLUGIN)
ksasl_auth_anon_la_LIBADD = $(LIB_KSYCOCA)
ksasl_auth_anon_la_SOURCES = auth_anon.cpp

noinst_HEADERS = auth_anon.h

service_DATA = kauthanonmodule.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO
