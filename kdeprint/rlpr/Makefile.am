AM_CPPFLAGS = -D_KDEPRINT_COMPILE

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)

kde_module_LTLIBRARIES = kdeprint_rlpr.la

kdeprint_rlpr_la_SOURCES = kmrlprfactory.cpp kmrlprmanager.cpp krlprprinterimpl.cpp kmrlpruimanager.cpp \
			      kmwrlpr.cpp kmproprlpr.cpp kmproxywidget.cpp kmconfigproxy.cpp
kdeprint_rlpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kdeprint_rlpr_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la
kdeprint_rlpr_la_METASOURCES = AUTO

noinst_HEADERS = kmrlprmanager.h krlprprinterimpl.h kmrlpruimanager.h kmwrlpr.h kmproprlpr.h \
		 kmproxywidget.h kmconfigproxy.h

entry_DATA = rlpr.print
entrydir = $(kde_datadir)/kdeprint/plugins
