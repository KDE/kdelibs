INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)

kde_module_LTLIBRARIES = libkdeprint_lpd.la

libkdeprint_lpd_la_SOURCES = kmlpdfactory.cpp kmlpdmanager.cpp klpdprinterimpl.cpp kmlpduimanager.cpp \
			     lpdtools.cpp gschecker.cpp
libkdeprint_lpd_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkdeprint_lpd_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la $(LIB_KDEUI)
libkdeprint_lpd_la_METASOURCES = AUTO

noinst_HEADERS = kmlpdfactory.h kmlpdmanager.h klpdprinterimpl.h kmlpduimanager.h lpdtools.h gschecker.h

bin_PROGRAMS = make_driver_db_lpd
make_driver_db_lpd_SOURCES = make_driver_db_lpd.c
make_driver_db_lpd_LDADD =

entry_DATA = lpd.print
entrydir = $(kde_datadir)/kdeprint/plugins
