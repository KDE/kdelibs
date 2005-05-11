lib_LTLIBRARIES = libweaverextensions.la

INCLUDES= -I../weaver $(all_includes)

include_HEADERS = weaverlogger.h weavervisualizer.h

noinst_HEADERS = visualizerwidget.h

libweaverextensions_la_SOURCES = weaverlogger.cpp weavervisualizer.cpp \
	visualizerwidget.cpp

libweaverextensions_la_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) -version-info 0:0


# libweaver_la_LIBADD = -lpam 

libweaverextensions_la_METASOURCES = AUTO

