lib_LTLIBRARIES = libkjava.la
libkjava_la_SOURCES = kjavaapplet.cpp kjavaappletcontext.cpp \
	kjavaappletserver.cpp kjavaappletwidget.cpp kjavaprocess.cpp

libkjava_la_METASOURCES = AUTO

include_HEADERS = kjavaapplet.h kjavaappletcontext.h \
	kjavaappletserver.h kjavaappletwidget.h kjavaprocess.h

libkjava_la_LDFLAGS = -version-info 1:0 -no-undefined
libkjava_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KIO)

INCLUDES = $(all_includes) -I$(top_srcdir)/kio/

data_DATA = kjava-classes.zip KJAS_GRAMMAR.txt kjava.policy
datadir = $(kde_datadir)/kjava/

SUBDIRS = tests
