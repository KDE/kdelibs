KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL)

noinst_LTLIBRARIES = libkjava.la
libkjava_la_SOURCES = kjavaapplet.cpp kjavaappletcontext.cpp \
	kjavaappletserver.cpp kjavaappletwidget.cpp kjavaprocess.cpp \
	kjavadownloader.cpp 

noinst_HEADERS = javaembed.h kjavaappletwidget.h kqeventutil.h kxeventutil.h \
                 kjavaapplet.h kjavaappletcontext.h \
                 kjavaappletserver.h kjavaprocess.h

libkjava_la_METASOURCES = AUTO

libkjava_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined
libkjava_la_LIBADD = $(LIB_KPARTS)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/khtml $(all_includes) 

data_DATA = kjava.jar kjava.policy
datadir = $(kde_datadir)/kjava/

SUBDIRS = . tests dummy
