# $Id$
INCLUDES = -I$(top_srcdir)/kio/ksasl -I$(top_srcdir)/kio $(all_includes)

lib_LTLIBRARIES = ksasl_auth_plain.la

ksasl_auth_plain_la_LDFLAGS = -module $(KDE_PLUGIN)
ksasl_auth_plain_la_LIBADD = $(LIB_KSYCOCA)
ksasl_auth_plain_la_SOURCES = auth_plain.cpp

noinst_HEADERS = auth_plain.h

service_DATA = kauthplainmodule.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO
