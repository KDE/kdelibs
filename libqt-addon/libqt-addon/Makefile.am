## Makefile.am for libqt-addon

SUBDIRS = tests

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
lib_LTLIBRARIES = libqt-addon.la

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# the library search path. 
libqt_addon_la_LDFLAGS = $(KDE_MT_LDFLAGS) $(all_libraries) -no-undefined -version-info 0:0
libqt_addon_la_LIBADD  = $(LIB_QT) $(LIB_IDN) $(LIB_SOCKET)
libqt_addon_la_SOURCES = qresolver.cpp qresolvermanager.cpp qresolverworkerbase.cpp \
		qsocketaddress.cpp qresolverstandardworkers.cpp \
		qsocketbase.cpp qsocketimpl.cpp qclientsocketbase.cpp \
		qstreamsocket.cpp qbufferedsocket.cpp qserversocket.cpp \
		qdatagramsocket.cpp qhttpproxysocketimpl.cpp

qt_addon_HEADERS = qtaddon.h qresolver.h qsocketaddress.h qsocketbase.h \
	qsocketimpl.h qclientsocketbase.h qstreamsocket.h qbufferedsocket.h \
	qserversocket.h qdatagramsocket.h qmulticastsocketimpl.h qmulticastsocket.h \
	qnetworkinterface.h qhttpproxysocketimpl.h
noinst_HEADERS = qresolver_p.h qresolverworkerbase.h qresolverstandardworkers_p.h

qt_addondir = $(includedir)/qt-addon

EXTRA_DIST = qsynchronizing.h qsynchronizing.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO
