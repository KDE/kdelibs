## Makefile.am for libqt-addon

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
noinst_LTLIBRARIES = libkdecorenetwork.la

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# the library search path. 
# convenience lib - no LDFLAGS or LIBADD !
# Note:
#  ksocketdevice.cpp must appear before any inclusion of syssocket.h
libkdecorenetwork_la_SOURCES = kresolver.cpp \
		kresolvermanager.cpp \
		kresolverworkerbase.cpp \
		ksocketaddress.cpp \
		kresolverstandardworkers.cpp \
		kreverseresolver.cpp \
		ksocketdevice.cpp \
		ksocketbase.cpp \
		kclientsocketbase.cpp \
		kstreamsocket.cpp \
		kbufferedsocket.cpp \
		kserversocket.cpp \
		kdatagramsocket.cpp \
		khttpproxysocketdevice.cpp \
		ksockssocketdevice.cpp

include_HEADERS = kresolver.h \
	kreverseresolver.h \
	ksocketaddress.h \
	ksocketbase.h \
	ksocketdevice.h \
	kclientsocketbase.h \
	kstreamsocket.h \
	kserversocket.h \
	kdatagramsocket.h \
	kmulticastsocketdevice.h \
	kmulticastsocket.h \
	knetworkinterface.h \
	khttpproxysocketdevice.h \
	ksockssocketdevice.h
noinst_HEADERS = kresolver_p.h kresolverworkerbase.h kresolverstandardworkers_p.h syssocket.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO
