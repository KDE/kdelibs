AM_CPPFLAGS = -D_KDEPRINT_COMPILE

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint $(all_includes)

kde_module_LTLIBRARIES = kdeprint_lpdunix.la

kdeprint_lpdunix_la_SOURCES = kmlpdunixfactory.cpp kmlpdunixmanager.cpp klpdunixprinterimpl.cpp kmlpdunixuimanager.cpp
kdeprint_lpdunix_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kdeprint_lpdunix_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la
kdeprint_lpdunix_la_METASOURCES = AUTO

noinst_HEADERS = kmlpdunixmanager.h klpdunixprinterimpl.h kmlpdunixuimanager.h

entry_DATA = lpdunix.print
entrydir = $(kde_datadir)/kdeprint/plugins
