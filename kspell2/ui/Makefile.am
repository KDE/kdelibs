METASOURCES = AUTO

lib_LTLIBRARIES = libkspell2.la

KDE_CXXFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT 
INCLUDES = -I$(top_srcdir)/kspell2 $(all_includes)

kspell2includedir = $(includedir)/kspell2
kspell2include_HEADERS = dialog.h highlighter.h configdialog.h \
	configwidget.h

libkspell2_la_SOURCES = kspell2ui.ui dialog.cpp \
	highlighter.cpp configui.ui configdialog.cpp \
	configwidget.cpp

libkspell2_la_LDFLAGS = -no-undefined  $(all_libraries)
libkspell2_la_LIBADD = ../libkspell2base.la
