INCLUDES = -I$(top_builddir)/kdedb -I$(top_srcdir)/kio $(all_includes)

METASOURCES = AUTO

KDE_ICON=AUTO

lib_LTLIBRARIES = libkdbui.la

libkdbui_la_SOURCES = plugininfodlg.cpp\
		connectionbase.ui\
		connectionwidget.cpp\
		connectiondlg.cpp\
		databasedlg.cpp\
		dblistview.cpp\
		recordsetview.cpp

libkdbui_la_LDFLAGS = -version-info 0:1:0 -no-undefined

libkdbui_la_LIBADD = ../kdbcore/libkdbcore.la


include_HEADERS = kdbui.h\
		plugininfodlg.h\
		connectionbase.h\
		connectionwidget.h\
		connectiondlg.h\
		databasedlg.h\
		dblistview.h\
		recordsetview.h

includedir = $(kde_includes)/kdb
