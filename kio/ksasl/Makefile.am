# $Id$
INCLUDES = $(all_includes)

SUBDIRS = . auth_CRAM-MD5 auth_PLAIN

lib_LTLIBRARIES = libksasl.la
libksasl_la_LDFLAGS = $(all_libraries) -version-info 1:0 -no-undefined
libksasl_la_LIBADD = $(LIB_KDECORE)
libksasl_la_SOURCES = saslcontext.cpp

ksaslincludedir = $(includedir)/kio/sasl
ksaslinclude_HEADERS = saslcontext.h saslmodule.h

METASOURCES = AUTO
