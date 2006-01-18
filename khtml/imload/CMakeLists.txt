kde4_header()

add_subdirectory( decoders ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = decoders . 
#
#noinst_LTLIBRARIES = libkhtmlimload.la
#
#libkhtmlimload_la_SOURCES = imageplane.cpp rawimageplane.cpp scaledimageplane.cpp \
#                            pixmapplane.cpp animprovider.cpp imagepainter.cpp \
#                            updater.cpp image.cpp imagemanager.cpp animtimer.cpp
#
#
##noinst_HEADERS = frame.h imageformat.h imageloaderprovider.h imageowner.h loaderdatabase.h pool.h\
##  tilecache.h updater.h image.h imageloader.h imagemanager.h imagetile.h  pixmaptile.h     tile.h\
##  tilestack.h vmmanager.h animtimer.h animprovider.h
#
#libkhtmlimload_la_LDFLAGS =  $(all_libraries)
#libkhtmlimload_la_LIBADD  = ./decoders/libdecoders.la $(LIB_KDECORE)
#
#INCLUDES = $(all_includes)
#
#METASOURCES = AUTO
