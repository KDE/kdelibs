INCLUDES = -I$(top_srcdir)/kutils -I$(top_srcdir) -I$(top_srcdir)/kmdi $(all_includes)

lib_LTLIBRARIES = libkmdi2.la
libkmdi2_la_LDFLAGS = $(all_libraries) -version-info 1:0
libkmdi2_la_LIBADD  = $(LIB_KPARTS) $(top_builddir)/kutils/libkutils.la

libkmdi2_la_SOURCES = mainwindow.cpp dockcontainer.cpp toolviewaccessor.cpp guiclient.cpp tabwidget.cpp

kmdiincludedir = $(includedir)/kmdi
kmdiinclude_HEADERS = global.h mainwindow.h toolviewaccessor.h tabwidget.h

METASOURCES = AUTO

include ../../admin/Doxyfile.am
