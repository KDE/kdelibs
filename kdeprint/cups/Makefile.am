LIB_CUPS = @LIB_CUPS@

SUBDIRS = cupsdconf

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kfile -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)

kde_module_LTLIBRARIES = libkdeprint_cups.la

libkdeprint_cups_la_SOURCES = cupsinfos.cpp ipprequest.cpp kmcupsfactory.cpp kmcupsjobmanager.cpp \
			      kmcupsmanager.cpp kmcupsuimanager.cpp kmpropbanners.cpp kmwbanners.cpp kmwipp.cpp \
			      kmwippselect.cpp kcupsprinterimpl.cpp kmcupsconfig.cpp kmcupsconfigwidget.cpp \
			      kmconfigcups.cpp kmconfigcupsdir.cpp kmwippprinter.cpp kmwfax.cpp kmwquota.cpp \
			      kmpropquota.cpp kmwusers.cpp kmpropusers.cpp kpschedulepage.cpp
libkdeprint_cups_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkdeprint_cups_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la $(LIB_KDEUI) $(LIB_KIO) $(LIB_CUPS)
libkdeprint_cups_la_METASOURCES = AUTO

noinst_HEADERS = cupsinfos.h ipprequest.h kmcupsfactory.h kmcupsjobmanager.h kmcupsmanager.h kmcupsuimanager.h \
		 kmpropbanners.h kmwbanners.h kmwipp.h kmwippselect.h kcupsprinterimpl.h kmcupsconfig.h \
		 kmcupsconfigwidget.h kmconfigcups.h kmconfigcupsdir.h kmwippprinter.h kmwfax.h kmwquota.h \
		 kmpropquota.h kmwusers.h kmpropusers.h kpschedulepage.h

bin_PROGRAMS = make_driver_db_cups cupsdoprint
make_driver_db_cups_SOURCES = make_driver_db_cups.c
make_driver_db_cups_LDADD = -lz

cupsdoprint_SOURCES = cupsdoprint.c
cupsdoprint_LDADD = $(LIB_CUPS)
cupsdoprint_LDFLAGS = $(all_libraries)

entry_DATA = cups.print
entrydir = $(kde_datadir)/kdeprint/plugins
