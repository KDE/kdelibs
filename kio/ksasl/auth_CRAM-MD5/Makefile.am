# $Id$
INCLUDES = -I$(top_srcdir)/kio/ksasl -I$(top_srcdir)/kio $(all_includes)

lib_LTLIBRARIES = ksasl_auth_cram.la

ksasl_auth_cram_la_LDFLAGS = -module $(KDE_PLUGIN)
ksasl_auth_cram_la_LIBADD = $(LIB_KSYCOCA)
ksasl_auth_cram_la_SOURCES = auth_cram.cpp

noinst_HEADERS = auth_cram.h

service_DATA = kauthcrammodule.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO
