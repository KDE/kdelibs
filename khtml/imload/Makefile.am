SUBDIRS = decoders . 

noinst_LTLIBRARIES = libkhtmlimload.la

libkhtmlimload_la_SOURCES = imagetile.cpp pixmaptile.cpp updater.cpp vmmanager.cpp imagemanager.cpp image.cpp frame.cpp animtimer.cpp animprovider.cpp

noinst_HEADERS = frame.h imageformat.h imageloaderprovider.h imageowner.h loaderdatabase.h pool.h\
  tilecache.h updater.h image.h imageloader.h imagemanager.h imagetile.h  pixmaptile.h     tile.h\
  tilestack.h vmmanager.h animtimer.h animprovider.h

libkhtmlimload_la_LDFLAGS =  $(all_libraries)
libkhtmlimload_la_LIBADD  = ./decoders/libdecoders.la $(LIB_KDECORE)

INCLUDES = $(all_includes)

METASOURCES = AUTO
