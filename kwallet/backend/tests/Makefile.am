INCLUDES = -I$(top_srcdir)/kwallet/backend -I$(top_srcdir)/kwallet/client $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = backendtest testbf testsha

METASOURCES = AUTO

LDADD = ../libkwalletbackend.la ../../client/libkwalletclient.la
backendtest_SOURCES = backendtest.cpp

testbf_SOURCES = testbf.cpp

testsha_SOURCES = testsha.cpp
