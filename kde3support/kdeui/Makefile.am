SUBDIRS = .

INCLUDES = -I$(top_srcdir)/kutils -I$(top_srcdir) -I$(top_srcdir)/kde3support/kdeui -I$(top_srcdir)/kde3support/kparts $(all_includes)

noinst_LTLIBRARIES = libkde3ui.la
libkde3ui_la_LDFLAGS = $(all_libraries)
libkde3ui_la_LIBADD  = $(LIB_KPARTS)

libkde3ui_la_SOURCES = k3dockwidget.cpp k3dockwidget_private.cpp k3popupmenu.cpp k3textedit.cpp k3colordrag.cpp

include_HEADERS = k3dockwidget.h k3popupmenu.h k3textedit.h k3colordrag.h

METASOURCES = AUTO

include ../../admin/Doxyfile.am
