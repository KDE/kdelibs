AM_CPPFLAGS = -D_KDEPRINT_COMPILE

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint $(all_includes)

kde_module_LTLIBRARIES = kdeprint_ext.la

kdeprint_ext_la_SOURCES = kmextfactory.cpp kmextmanager.cpp kextprinterimpl.cpp kmextuimanager.cpp
kdeprint_ext_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kdeprint_ext_la_LIBADD = $(top_builddir)/kdeprint/libkdeprint.la
kdeprint_ext_la_METASOURCES = AUTO

noinst_HEADERS = kmextmanager.h kextprinterimpl.h kmextuimanager.h

entry_DATA = ext.print
entrydir = $(kde_datadir)/kdeprint/plugins
