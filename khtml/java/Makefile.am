lib_LTLIBRARIES = libkjava.la
libkjava_la_SOURCES = kjavaapplet.cpp kjavaappletcontext.cpp \
	kjavaappletserver.cpp kjavaappletwidget.cpp kjavaprocess.cpp

libkjava_la_METASOURCES = kjavaprocess.moc kjavaappletwidget.moc \
	kjavaappletcontext.moc kjavaapplet.moc kjavaappletserver.moc

include_HEADERS = kjavaapplet.h kjavaappletcontext.h \
	kjavaappletserver.h kjavaappletwidget.h kjavaprocess.h

libkjava_la_LDFLAGS = -version-info 1:0
libkjava_la_LIBADD = ../../kdecore/libkdecore.la

INCLUDES = $(all_includes)

