# $Id$

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui -I$(top_srcdir)/kio \
	   -I$(top_srcdir)/kfile -I$(top_srcdir)/kparts -I$(top_srcdir)/kspell -I$(top_srcdir)/libltdl $(all_includes)

lib_LTLIBRARIES = libkwrite.la

libkwrite_la_SOURCES = kwrite.cpp kwrite_view.cpp highlight.cpp \
	kwrite_part.cpp kwrite_factory.cpp kwrite_doc.cpp \
	kwdialog.cpp undohistory.cpp ktextprint.cpp prefdialog.cpp \
	kwrite_keys.cpp

libkwrite_la_LIBADD = -L$(top_srcdir)/kparts -L$(top_srcdir)/kfile -L$(top_srcdir)/kspell ../kparts/libkparts.la ../kfile/libkfile.la ../kspell/libkspell.la
libkwrite_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined

METASOURCES = AUTO

kwriteincludedir = $(includedir)/kwrite
kwriteinclude_HEADERS = kwrite_part.h kwrite_factory.h kwrite.h kwrite_view.h \
	kwrite_doc.h highlight.h

rcdir = $(kde_datadir)/kwrite
rc_DATA = kwrite.rc kwrite_browser.rc

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = kwrite_part.desktop

messages: rc.cpp
	$(XGETTEXT) -ki18nop *.cpp -o $(podir)/libkwrite.pot

