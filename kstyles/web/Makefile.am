INCLUDES              = $(all_includes)
lib_LTLIBRARIES       = webstyle.la
webstyle_la_LDFLAGS   = $(all_libraries) -module -avoid-version -no-undefined 
webstyle_la_LIBADD    = $(LIB_KDEUI)
webstyle_la_SOURCES   = webstyle.cpp plugin.cpp
noinst_HEADERS        = webstyle.h 

kstylewebdata_DATA = web.themerc
kstylewebdatadir = $(kde_datadir)/kstyle/themes
EXTRA_DIST = $(kstylewebdata_DATA)
