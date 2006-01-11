check_PROGRAMS = kdehwtest

TESTS = kdehwtest

INCLUDES = -I$(srcdir)/../kdehw $(all_includes)
AM_LDFLAGS = $(all_libraries)

LDFLAGS = $(KDE_RPATH) $(all_libraries)
LDADD = $(LIB_KDECORE) -lQtTest_debug ../kdehw/libkdehw.la ../kdehw/ifaces/libkdehwifaces.la

kdehwtest_SOURCES = kdehwtest.cpp fakedevice.cpp fakemanager.cpp

METASOURCES = AUTO

