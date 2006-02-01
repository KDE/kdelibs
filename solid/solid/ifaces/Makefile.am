INCLUDES = $(all_includes)

lib_LTLIBRARIES  = libkdehwifaces.la 
libkdehwifaces_la_LIBADD = $(LIB_QTCORE)
libkdehwifaces_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)

include_HEADERS = devicemanager.h device.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h

METASOURCES = AUTO

libkdehwifaces_la_SOURCES = devicemanager.cpp device.cpp capability.cpp processor.cpp block.cpp storage.cpp cdrom.cpp volume.cpp opticaldisc.cpp

