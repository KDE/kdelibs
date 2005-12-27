INCLUDES= $(all_includes)
SUBDIRS = . 
KDE_CXXFLAGS = -UQT3_SUPPORT

lib_LTLIBRARIES = libkdeeduplot.la

libkdeeduplotincludedir = $(includedir)/libkdeedu
libkdeeduplotinclude_HEADERS = kplotobject.h kplotaxis.h kplotwidget.h

libkdeeduplot_la_SOURCES = kplotobject.cpp kplotaxis.cpp kplotwidget.cpp

libkdeeduplot_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
libkdeeduplot_la_LIBADD = $(LIB_KDECORE) $(LIB_QT)

METASOURCES = AUTO

