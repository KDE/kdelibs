kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kutils ${CMAKE_SOURCE_DIR}/kde3support/kparts ${CMAKE_SOURCE_DIR}/kde3support/kdeui ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = .
#
#INCLUDES = -I$(top_srcdir)/kutils -I$(top_srcdir) -I$(top_srcdir)/kde3support/kparts -I$(top_srcdir)/kde3support/kdeui $(all_includes)
#
#noinst_LTLIBRARIES = libk3parts.la
#libk3parts_la_LDFLAGS = $(all_libraries)
#libk3parts_la_LIBADD  = $(LIB_KPARTS)
#
#libk3parts_la_SOURCES = dockmainwindow3.cpp
#
#include_HEADERS = dockmainwindow3.h
#
#METASOURCES = AUTO
#
#include ../../admin/Doxyfile.am
