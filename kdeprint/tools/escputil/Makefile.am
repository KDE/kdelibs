INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kdeprint_tool_escputil.la
kdeprint_tool_escputil_la_SOURCES = escpwidget.cpp
kdeprint_tool_escputil_la_METASOURCES = AUTO
kdeprint_tool_escputil_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kdeprint_tool_escputil_la_LIBADD = $(top_builddir)/kdeui/libkdeui.la

noinst_HEADERS = escpwidget.h

service_DATA = escputil.desktop
servicedir = $(kde_datadir)/kdeprint/tools
