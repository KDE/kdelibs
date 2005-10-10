INCLUDES = $(all_includes)

lib_LTLIBRARIES  = libkdehw.la 
libkdehw_la_LIBADD = $(LIB_QTCORE) $(LIB_QT) $(LIB_KDECORE) -lQtDBUS_debug
libkdehw_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)

include_HEADERS = halmanager.h haldevice.h

METASOURCES = AUTO

libkdehw_la_SOURCES = halmanager.cpp haldevice.cpp

