METASOURCES = AUTO

noinst_LTLIBRARIES = libkspell2ui.la

KDE_CXXFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT 
INCLUDES = -I$(top_srcdir)/kspell2 $(all_includes)

kspell2includedir = $(includedir)/kspell2
kspell2include_HEADERS = dialog.h highlighter.h configdialog.h \
	configwidget.h

libkspell2ui_la_SOURCES = kspell2ui.ui dialog.cpp \
	highlighter.cpp configui.ui configdialog.cpp \
	configwidget.cpp

libkspell2ui_la_LDFLAGS = -no-undefined  $(all_libraries)
