# $Id$

SUBDIRS = . auth_CRAM-MD5 auth_PLAIN

lib_LTLIBRARIES = libksasl.la
libksasl_la_LDFLAGS = $(all_libraries) -version-info 3:0 -no-undefined
libksasl_la_LIBADD = -lkdecore
libksasl_la_SOURCES = saslcontext.cpp

METASOURCES = AUTO
