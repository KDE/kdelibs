INCLUDES = -I$(top_srcdir)/kwallet/backend -I$(top_srcdir)/kwallet/client $(all_includes)

check_PROGRAMS = backendtest

METASOURCES = AUTO

LDADD = ../libkwalletbackend.la ../../client/libkwalletclient.la
backendtest_SOURCES = backendtest.cpp
backendtest_LDADD = ../libkwalletbackend.la ../../client/libkwalletclient.la
backendtest_LDFLAGS = $(all_libraries)

