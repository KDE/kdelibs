# $Id$

INCLUDES = $(all_includes) -I$(top_srcdir)

lib_LTLIBRARIES = libkwrite.la

libkwrite_la_SOURCES = kwrite.cpp kwrite_view.cpp highlight.cpp \
	kwrite_shell.cpp kwrite_part.cpp kwrite_factory.cpp kwrite_doc.cpp \
	kwdialog.cpp undohistory.cpp ktextprint.cpp prefdialog.cpp \
	kguicommand.cpp

libkwrite_la_LIBADD = $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KSPELL)
libkwrite_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined

METASOURCES = AUTO

kwriteincludedir = $(includedir)/kwrite
kwriteinclude_HEADERS = kwrite_part.h kwrite_factory.h kwrite.h kwrite_view.h \
	kwrite_doc.h highlight.h

rcdir = $(kde_datadir)/kwrite
rc_DATA = kwrite.rc kwrite_shell.rc kwrite_browser.rc

KDE_ICON = kwrite

messages: rc.cpp
	$(XGETTEXT) -ki18nop *.cpp -o $(podir)/kwrite.pot

