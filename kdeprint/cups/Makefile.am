SUBDIRS = cupsdconf2

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kfile -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)

kde_module_LTLIBRARIES = kdeprint_cups.la

kdeprint_cups_la_SOURCES = cupsinfos.cpp ipprequest.cpp kmcupsfactory.cpp kmcupsjobmanager.cpp \
			      kmcupsmanager.cpp kmcupsuimanager.cpp kmpropbanners.cpp kmwbanners.cpp kmwipp.cpp \
			      kmwippselect.cpp kcupsprinterimpl.cpp kmcupsconfigwidget.cpp \
			      kmconfigcups.cpp kmconfigcupsdir.cpp kmwippprinter.cpp kmwfax.cpp kmwquota.cpp \
			      kmpropquota.cpp kmwusers.cpp kmpropusers.cpp kpschedulepage.cpp image.cpp \
				  imagepreview.cpp imageposition.cpp kpimagepage.cpp \
				  kptextpage.cpp kphpgl2page.cpp cupsaddsmb2.cpp kmwother.cpp ippreportdlg.cpp \
				  kptagspage.cpp
kdeprint_cups_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kdeprint_cups_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la $(top_builddir)/kio/libkio.la $(LIB_CUPS) $(top_builddir)/kdecore/libkdefakes.la
kdeprint_cups_la_METASOURCES = AUTO

noinst_HEADERS = cupsinfos.h ipprequest.h kmcupsfactory.h kmcupsjobmanager.h kmcupsmanager.h kmcupsuimanager.h \
		 kmpropbanners.h kmwbanners.h kmwipp.h kmwippselect.h kcupsprinterimpl.h \
		 kmcupsconfigwidget.h kmconfigcups.h kmconfigcupsdir.h kmwippprinter.h kmwfax.h kmwquota.h \
		 kmpropquota.h kmwusers.h kmpropusers.h kpschedulepage.h imagepreview.h imageposition.h \
		 kpimagepage.h kptextpage.h kphpgl2page.h cupsaddsmb2.h \
		 kmwother.h ippreportdlg.h kptagspage.h

bin_PROGRAMS = make_driver_db_cups cupsdoprint
make_driver_db_cups_SOURCES = make_driver_db_cups.c
make_driver_db_cups_LDADD = -lz ../libdriverparse.a $(top_builddir)/kdecore/libkdefakes.la $(LIBDL)

cupsdoprint_SOURCES = cupsdoprint.c
cupsdoprint_LDADD = $(LIB_CUPS) $(top_builddir)/kdecore/libkdefakes.la
cupsdoprint_LDFLAGS = $(all_libraries)

entry_DATA = cups.print
entrydir = $(kde_datadir)/kdeprint/plugins

misc_DATA = preview.png preview-mini.png
miscdir = $(kde_datadir)/kdeprint
