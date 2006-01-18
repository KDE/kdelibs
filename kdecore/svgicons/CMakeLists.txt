kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/.. $(AGG_CFLAGS) $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#noinst_LTLIBRARIES = libkdesvgicons.la
#
#include_HEADERS = ksvgiconengine.h
#libkdesvgicons_la_SOURCES = ksvgiconengine.cpp ksvgiconpainter.cpp
