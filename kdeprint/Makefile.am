
SUBDIRS = . management lpdunix $(CUPSSUBDIR) lpd rlpr ext filters pics tests

INCLUDES= -I$(top_srcdir)/kio -I$(top_srcdir)/kio/kded -I$(top_srcdir)/kfile -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkded_kdeprintd.la

libkded_kdeprintd_la_SOURCES = kdeprintd.skel kdeprintd.cpp kprintprocess.cpp
libkded_kdeprintd_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO)
libkded_kdeprintd_la_LDFLAGS = $(all_libraries) -module -avoid-version
#libkded_kdeprintd_la_METASOURCES = AUTO

lib_LTLIBRARIES = libkdeprint.la

libkdeprint_la_SOURCES = kmfactory.cpp kmjob.cpp kmjobmanager.cpp kmmanager.cpp kmprinter.cpp \
		         kmuimanager.cpp kmvirtualmanager.cpp driver.cpp driveritem.cpp droptionview.cpp \
			 driverview.cpp maticscanner.cpp maticparser.cpp matic.cpp kmdbentry.cpp \
		         kprintpreview.cpp kmfiltermanager.cpp kprintdialog.cpp kprinter.cpp kprinterimpl.cpp kprintdialogpage.cpp \
			 kpcopiespage.cpp kpdriverpage.cpp kprinterpropertydialog.cpp kpgeneralpage.cpp kpqtpage.cpp \
			 kmthreadjob.cpp kpipeprocess.cpp kprintfilter.cpp kpfilterpage.cpp \
			 kmspecialmanager.cpp kdeprintcheck.cpp plugincombobox.cpp kpreloadobject.cpp
libkdeprint_la_LDFLAGS = $(all_libraries) -version-info 0:1 -no-undefined
libkdeprint_la_LIBADD = $(LIB_KPARTS) $(LIBZ)
libkdeprint_la_METASOURCES = AUTO

noinst_HEADERS = kmdbentry.h kmfactory.h kmjob.h kmjobmanager.h kmmanager.h kmobject.h kmprinter.h \
		 kmuimanager.h kmvirtualmanager.h driver.h driveritem.h droptionview.h \
		 driverview.h matic.h kprintpreview.h kprinterimpl.h kprintdialog.h \
		 kpcopiespage.h kpdriverpage.h kprinterpropertydialog.h kpgeneralpage.h \
		 kpqtpage.h kprintprocess.h kmthreadjob.h kpipeprocess.h kdeprintd.h kprintfilter.h \
		 kmfiltermanager.h kpfilterpage.h kmspecialmanager.h kdeprintcheck.h plugincombobox.h

include_HEADERS = kprinter.h

kdeprint_HEADERS = kprintdialogpage.h kpreloadobject.h
kdeprintdir = $(includedir)/kdeprint

EXTRA_DIST = maticparser.yy maticscanner.ll kprintpreviewui.rc

# installation of UI files
uidata_DATA = kprintpreviewui.rc
uidatadir = $(kde_confdir)/ui

# Test page
printdata_DATA = testprint.ps specials.desktop
printdatadir = $(kde_datadir)/kdeprint

# Services
servicesdir = $(kde_servicesdir)/kded
services_DATA = kdeprintd.desktop

# parser and scanner
parser:
	bison -d -p matic $(srcdir)/maticparser.yy -o $(srcdir)/maticparser.cpp
	flex -omaticscanner.cpp -Pmatic $(srcdir)/maticscanner.ll

noinst_DATA = kdeprint

kdeprint:
	$(LN_S) $(srcdir)/management kdeprint

