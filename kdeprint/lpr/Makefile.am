INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)

kde_module_LTLIBRARIES = libkdeprint_lpr.la

libkdeprint_lpr_la_SOURCES = printcapreader.cpp lprhandler.cpp \
			     matichandler.cpp klprfactory.cpp kmlprmanager.cpp \
			     kmlpruimanager.cpp lpchelper.cpp lpqhelper.cpp \
			     kmlprjobmanager.cpp matichelper.cpp \
			     klprprinterimpl.cpp printcapentry.cpp \
			     lprsettings.cpp
libkdeprint_lpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkdeprint_lpr_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la $(LIB_KDEUI)
libkdeprint_lpr_la_METASOURCES = AUTO

noinst_HEADERS = printcapentry.h printcapreader.h lprhandler.h \
		 matichandler.h klprfactory.h kmlprmanager.h \
		 kmlpruimanager.h lpchelper.h lpqhelper.h \
		 kmlprjobmanager.h matichelper.h klprprinterimpl.h \
		 lprsettings.h

#bin_PROGRAMS = make_driver_db_lpd
#make_driver_db_lpd_SOURCES = make_driver_db_lpd.c
#make_driver_db_lpd_LDADD =

entry_DATA = lpr.print
entrydir = $(kde_datadir)/kdeprint/plugins
