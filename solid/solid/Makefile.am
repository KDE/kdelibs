INCLUDES = $(all_includes)

SUBDIRS = . ifaces

lib_LTLIBRARIES  = libkdehw.la 
libkdehw_la_LIBADD = $(LIB_QTCORE) $(LIB_QT) $(LIB_KDECORE) -lQtDBUS_debug ifaces/libkdehwifaces.la
libkdehw_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)

include_HEADERS = device.h devicemanager.h halmanager.h haldevice.h

METASOURCES = AUTO

libkdehw_la_SOURCES = device.cpp devicemanager.cpp halmanager.cpp haldevice.cpp

