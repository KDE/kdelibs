INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)

kde_module_LTLIBRARIES = libkdeprint_rlpr.la

libkdeprint_rlpr_la_SOURCES = kmrlprfactory.cpp kmrlprmanager.cpp krlprprinterimpl.cpp kmrlpruimanager.cpp \
			      kmwrlpr.cpp kmproprlpr.cpp kmproxywidget.cpp kmconfigproxy.cpp kmproxydialog.cpp
libkdeprint_rlpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkdeprint_rlpr_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la
libkdeprint_rlpr_la_METASOURCES = AUTO

noinst_HEADERS = kmrlprfactory.h kmrlprmanager.h krlprprinterimpl.h kmrlpruimanager.h kmwrlpr.h kmproprlpr.h \
		 kmproxywidget.h kmconfigproxy.h kmproxydialog.h

entry_DATA = rlpr.print
entrydir = $(kde_datadir)/kdeprint/plugins
