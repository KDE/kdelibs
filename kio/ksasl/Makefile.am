# $Id$

INCLUDES = -I$(top_srcdir)/kio $(all_includes)

SUBDIRS = . auth_ANONYMOUS auth_CRAM-MD5 auth_DIGEST-MD5 auth_PLAIN

lib_LTLIBRARIES = libksasl.la
libksasl_la_LDFLAGS = -module $(KDE_PLUGIN)
libksasl_la_LIBADD = $(LIB_KSYCOCA)
libksasl_la_SOURCES = saslcontext.cpp

ksaslincludedir = $(includedir)/kio/ksasl
ksaslinclude_HEADERS = saslcontext.h saslmodule.h

kde_servicetypes_DATA = ksasl.desktop

METASOURCES = AUTO
