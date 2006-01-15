kde4_header()

add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############

install_files( ${KDE4_CONFIG_DIR} FILES ipv6blacklist )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for libqt-addon
#
#SUBDIRS = tests
#
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#noinst_LTLIBRARIES = libkdecorenetwork.la
#
## set the include path for X, qt and KDE
#INCLUDES = $(all_includes)
#
## the library search path. 
## convenience lib - no LDFLAGS or LIBADD !
## Note:
##  ksocketdevice.cpp must appear before any inclusion of ksocketdevice.h
#libkdecorenetwork_la_SOURCES = kresolver.cpp \
#		kresolvermanager.cpp \
#		kresolverworkerbase.cpp \
#		ksocketaddress.cpp \
#		kresolverstandardworkers.cpp \
#		kreverseresolver.cpp \
#		ksocketdevice.cpp \
#		ksocketbase.cpp \
#		kclientsocketbase.cpp \
#		kstreamsocket.cpp \
#		kserversocket.cpp \
#		kdatagramsocket.cpp \
#		khttpproxysocketdevice.cpp \
#		ksockssocketdevice.cpp \
#		kbufferedsocket.cpp \
#		ksocketbuffer.cpp\
#		knetworkinterface.cpp
#
#include_HEADERS = kresolver.h \
#	kreverseresolver.h \
#	ksocketaddress.h \
#	ksocketbase.h \
#	ksocketdevice.h \
#	kclientsocketbase.h \
#	kstreamsocket.h \
#	kserversocket.h \
#	kdatagramsocket.h \
#	kmulticastsocketdevice.h \
#	kmulticastsocket.h \
#	knetworkinterface.h \
#	khttpproxysocketdevice.h \
#	ksockssocketdevice.h \
#	kbufferedsocket.h \
#	kiobuffer.h
#	
#noinst_HEADERS = kresolver_p.h \
#	kresolverworkerbase.h \
#	kresolverstandardworkers_p.h \
#	ksocketbuffer_p.h \
#	syssocket.h
#
#configdir = $(kde_confdir)
#config_DATA = ipv6blacklist
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
