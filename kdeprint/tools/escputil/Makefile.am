INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = libescputil.la
libescputil_la_SOURCES = escpwidget.cpp
libescputil_la_METASOURCES = AUTO
libescputil_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libescputil_la_LIBADD = $(LIB_KDEUI)

noinst_HEADERS = escpwidget.h

service_DATA = escputil.desktop
servicedir = $(kde_datadir)/kdeprint/tools
