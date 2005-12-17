INCLUDES = $(all_includes)

SUBDIRS = . ifaces

lib_LTLIBRARIES  = libkdehw.la 
libkdehw_la_LIBADD = $(LIB_QTCORE) $(LIB_QT) $(LIB_KDECORE) $(LIB_KIO) ifaces/libkdehwifaces.la
libkdehw_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)

include_HEADERS = device.h devicemanager.h

METASOURCES = AUTO

libkdehw_la_SOURCES = device.cpp devicemanager.cpp

kde_servicetypes_DATA = kdehwdevicemanager.desktop

