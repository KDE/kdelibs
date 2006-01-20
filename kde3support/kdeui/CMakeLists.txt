kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kutils ${CMAKE_SOURCE_DIR}/kde3support/kdeui ${CMAKE_SOURCE_DIR}/kde3support/kparts ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -DKDE3_SUPPORT
#
#INCLUDES = -I$(top_srcdir)/kutils -I$(top_srcdir) -I$(top_srcdir)/kde3support/kdeui -I$(top_srcdir)/kde3support/kparts $(all_includes)
#
#noinst_LTLIBRARIES = libkde3ui.la
#libkde3ui_la_LDFLAGS = $(all_libraries)
#libkde3ui_la_LIBADD  = $(LIB_KPARTS)
#
#libkde3ui_la_SOURCES = k3dockwidget.cpp k3dockwidget_private.cpp k3popupmenu.cpp k3textedit.cpp k3colordrag.cpp k3syntaxhighlighter.cpp
#
#include_HEADERS = k3dockwidget.h k3popupmenu.h k3textedit.h k3colordrag.h k3syntaxhighlighter.h
#
#METASOURCES = AUTO
#
#include ../../admin/Doxyfile.am
