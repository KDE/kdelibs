INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)

kde_module_LTLIBRARIES = kdeprint_lpr.la

kdeprint_lpr_la_SOURCES = printcapreader.cpp lprhandler.cpp \
			     matichandler.cpp klprfactory.cpp kmlprmanager.cpp \
			     kmlpruimanager.cpp lpchelper.cpp lpqhelper.cpp \
			     kmlprjobmanager.cpp matichelper.cpp \
			     klprprinterimpl.cpp printcapentry.cpp \
			     lprsettings.cpp kmconfiglpr.cpp apshandler.cpp \
			     lprngtoolhandler.cpp susehandler.cpp susehelper.cpp \
			     editentrydialog.cpp
kdeprint_lpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kdeprint_lpr_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la $(LIB_KDEUI) $(top_builddir)/kdecore/libkdefakes.la
kdeprint_lpr_la_METASOURCES = AUTO

noinst_HEADERS = printcapentry.h printcapreader.h lprhandler.h \
		 matichandler.h klprfactory.h kmlprmanager.h \
		 kmlpruimanager.h lpchelper.h lpqhelper.h \
		 kmlprjobmanager.h matichelper.h klprprinterimpl.h \
		 lprsettings.h kmconfiglpr.h apshandler.h \
		 lprngtoolhandler.h susehandler.h susehelper.h \
		 editentrydialog.h

bin_PROGRAMS = make_driver_db_lpr
make_driver_db_lpr_SOURCES = make_driver_db_lpr.c
make_driver_db_lpr_LDADD = $(top_builddir)/kdecore/libkdefakes.la ../libdriverparse.a

entry_DATA = lpr.print
entrydir = $(kde_datadir)/kdeprint/plugins

miscdata_DATA = apsdriver1 apsdriver2 lprngtooldriver1
miscdatadir = $(kde_datadir)/kdeprint
