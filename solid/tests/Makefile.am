bin_PROGRAMS = kdehw_test

INCLUDES = -I$(srcdir)/../kdehw $(all_includes)
AM_LDFLAGS = $(all_libraries)

kdehw_test_SOURCES = kdehw_test.cpp fakedevice.cpp fakemanager.cpp

kdehw_test_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kdehw_test_LDADD = $(LIB_KDECORE) ../kdehw/libkdehw.la ../kdehw/ifaces/libkdehwifaces.la

METASOURCES = AUTO

