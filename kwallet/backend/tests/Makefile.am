INCLUDES = -I$(top_srcdir)/kwallet/backend -I$(top_srcdir)/kwallet/client $(all_includes)

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = backendtest

noinst_HEADERS =

METASOURCES = AUTO

LDADD = ../libkwalletbackend.la ../../client/libkwalletclient.la
backendtest_SOURCES = backendtest.cpp

