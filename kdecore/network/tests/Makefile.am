## Makefile.am for libqt-addon

INCLUDES = -I$(srcdir)/.. $(all_includes)

check_PROGRAMS = resolution socketimpltest streamsockettest serversockettest
LDADD = ../../libkdecore.la

resolution_SOURCES = resolution.cpp
socketimpltest_SOURCES = socketimpltest.cpp
streamsockettest_SOURCES = streamsockettest.cpp
serversockettest_SOURCES = serversockettest.cpp

noinst_HEADERS = resolution.h

METASOURCES = AUTO
