kde4_header()



########### next target ###############


########### install files ###############
install_files( /include FILES ksvgiconengine.h )


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/.. $(AGG_CFLAGS) $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#noinst_LTLIBRARIES = libkdesvgicons.la
#
#include_HEADERS = ksvgiconengine.h
#libkdesvgicons_la_SOURCES = ksvgiconengine.cpp ksvgiconpainter.cpp
