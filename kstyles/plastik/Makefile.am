AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes)

stylercdir = $(kde_datadir)/kstyle/themes
stylerc_DATA = plastik.themerc

EXTRA_DIST = $(stylerc_DATA)

noinst_HEADERS = plastik.h misc.h

kde_style_LTLIBRARIES = plastik.la
plastik_la_SOURCES = plastik.cpp misc.cpp
plastik_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
plastik_la_LIBADD = ../../kdefx/libkdefx.la
plastik_la_METASOURCES = AUTO

DISTCLEANFILES = $(plastik_la_METASOURCES)
