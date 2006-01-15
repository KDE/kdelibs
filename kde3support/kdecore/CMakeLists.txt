kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdecore ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = .
#
#INCLUDES = -I$(top_srcdir)/kdecore -I$(top_srcdir) $(all_includes)
#
#noinst_LTLIBRARIES = libkde3core.la
#libkde3core_la_LDFLAGS = $(all_libraries)
##libkde3core_la_LIBADD  = $(LIB_KPARTS)
#
#libkde3core_la_SOURCES = k3shortcut.cpp k3keysequence.cpp k3shortcutmenu.cpp \
#	k3urldrag.cpp k3multipledrag.cpp
#include_HEADERS = k3shortcut.h k3keysequence.h k3shortcutmenu.h \
#	k3urldrag.h k3multipledrag.h
#
#METASOURCES = AUTO
#
#include ../../admin/Doxyfile.am
