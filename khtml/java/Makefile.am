KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL)

lib_LTLIBRARIES = libkjava.la
libkjava_la_SOURCES = kqeventutil.cpp kxeventutil.cpp kjavaapplet.cpp kjavaappletcontext.cpp \
	kjavaappletserver.cpp kjavaappletwidget.cpp kjavaprocess.cpp \
	kjavadownloader.cpp javaembed.cpp 

noinst_HEADERS = javaembed.h kjavaappletwidget.h kqeventutil.h kxeventutil.cpp

libkjava_la_METASOURCES = AUTO

include_HEADERS = kjavaapplet.h kjavaappletcontext.h \
	kjavaappletserver.h kjavaprocess.h

libkjava_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 1:0 -no-undefined
libkjava_la_LIBADD = $(LIB_KPARTS)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/khtml $(all_includes) 

data_DATA = kjava.jar kjava.policy
datadir = $(kde_datadir)/kjava/

SUBDIRS = . tests
