INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint $(all_includes)

kde_module_LTLIBRARIES = libkdeprint_ext.la

libkdeprint_ext_la_SOURCES = kmextfactory.cpp kmextmanager.cpp kextprinterimpl.cpp kmextuimanager.cpp
libkdeprint_ext_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkdeprint_ext_la_LIBADD = $(top_builddir)/kdeprint/libkdeprint.la
libkdeprint_ext_la_METASOURCES = AUTO

noinst_HEADERS = kmextfactory.h kmextmanager.h kextprinterimpl.h kmextuimanager.h

entry_DATA = ext.print
entrydir = $(kde_datadir)/kdeprint/plugins
