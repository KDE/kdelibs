INCLUDES              = $(all_includes)
lib_LTLIBRARIES       = webstyle.la
webstyle_la_LDFLAGS   = $(all_libraries) -module -avoid-version -no-undefined 
webstyle_la_LIBADD    = $(top_builddir)/kdeui/libkdeui.la
webstyle_la_SOURCES   = webstyle.cpp plugin.cpp
noinst_HEADERS        = webstyle.h 

data_DATA = web.themerc
datadir = $(kde_datadir)/kstyle/themes
EXTRA_DIST = $(data_DATA)
