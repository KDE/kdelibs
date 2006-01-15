kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio/bookmarks ${CMAKE_SOURCE_DIR}/kde3support/kdecore ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kio/bookmarks -I$(top_srcdir)/kde3support/kdecore $(all_includes)
#
#noinst_LTLIBRARIES = libk3io.la
#libk3io_la_LDFLAGS = $(all_libraries)
#libk3io_la_LIBADD  = $(LIB_KPARTS)
#
#libk3io_la_SOURCES = k3bookmarkdrag.cpp
#
#include_HEADERS = k3bookmarkdrag.h
#
#METASOURCES = AUTO
#
#include ../../admin/Doxyfile.am
