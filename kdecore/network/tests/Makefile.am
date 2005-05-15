## Makefile.am for libqt-addon

INCLUDES = -I$(top_srcdir)/libqt-addon/libqt-addon -I$(qt_includes)

check_PROGRAMS = resolution synchro socketimpltest streamsockettest serversockettest
LDADD = ../libqt-addon.la $(LIB_QT)
KDE_CXXFLAGS = -O0 -g

resolution_SOURCES = resolution.cpp
synchro_SOURCES = synchro.cpp
socketimpltest_SOURCES = socketimpltest.cpp
streamsockettest_SOURCES = streamsockettest.cpp
serversockettest_SOURCES = serversockettest.cpp

resolution.moc: resolution.cpp

%.moc: %.cpp
	$(MOC) -o $@ $^ 

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO
