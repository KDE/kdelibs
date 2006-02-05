METASOURCES = AUTO

lib_LTLIBRARIES = libkspell2.la
# Put most of the code in a noinst lib, for the unit tests to be able to use internal classes.
noinst_LTLIBRARIES = libkspell2_noinst.la

KDE_CXXFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT
INCLUDES = -I$(top_srcdir)/kspell2 $(all_includes)

kspell2includedir = $(includedir)/kspell2
kspell2include_HEADERS = dialog.h highlighter.h configdialog.h \
	configwidget.h

libkspell2_noinst_la_SOURCES = kspell2ui.ui3 configwidget.cpp \
	highlighter.cpp configui.ui3 configdialog.cpp

dialog.lo: kspell2ui.h

libkspell2_la_SOURCES = dialog.cpp
libkspell2_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkspell2_la_LIBADD = libkspell2_noinst.la ../libkspell2base.la
