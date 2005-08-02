## Makefile.am for libqt-addon

INCLUDES = -I$(srcdir)/.. $(all_includes)

check_PROGRAMS = netif resolution socketdevicetest streamsockettest serversockettest 
LDADD = ../../libkdecore.la

netif_SOURCES = netif.cpp
resolution_SOURCES = resolution.cpp
socketdevicetest_SOURCES = socketdevicetest.cpp
streamsockettest_SOURCES = streamsockettest.cpp
serversockettest_SOURCES = serversockettest.cpp

noinst_HEADERS = resolution.h netif.h

METASOURCES = AUTO
