INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint $(all_includes)

kde_module_LTLIBRARIES = libkdeprint_lpdunix.la

libkdeprint_lpdunix_la_SOURCES = kmlpdunixfactory.cpp kmlpdunixmanager.cpp klpdunixprinterimpl.cpp
libkdeprint_lpdunix_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkdeprint_lpdunix_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la
libkdeprint_lpdunix_la_METASOURCES = AUTO

noinst_HEADERS = kmlpdunixfactory.h kmlpdunixmanager.h klpdunixprinterimpl.h

entry_DATA = lpdunix.print
entrydir = $(kde_datadir)/kdeprint/plugins
