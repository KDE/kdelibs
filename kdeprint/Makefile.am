CUPSSUBDIR = @CUPSSUBDIR@

SUBDIRS = . management $(CUPSSUBDIR) lpd

INCLUDES= -I$(top_srcdir)/kio -I$(top_srcdir)/kfile -I$(top_srcdir) $(all_includes) 

lib_LTLIBRARIES = libkdeprint.la

libkdeprint_la_SOURCES = kmfactory.cpp kmjob.cpp kmjobmanager.cpp kmmanager.cpp kmprinter.cpp \
		         kmuimanager.cpp kmvirtualmanager.cpp driver.cpp driveritem.cpp droptionview.cpp \
			 driverview.cpp maticscanner.cpp maticparser.cpp matic.cpp kmdbentry.cpp \
		         kprintpreview.cpp kprinter.cpp kprinterimpl.cpp kprintdialog.cpp kprintdialogpage.cpp \
			 kpcopiespage.cpp kpdriverpage.cpp kprinterpropertydialog.cpp kpgeneralpage.cpp kpqtpage.cpp
libkdeprint_la_LDFLAGS = $(all_libraries) -version 0:1
libkdeprint_la_LIBADD = $(LIB_KPARTS) $(LIB_KFILE) -lz
libkdeprint_la_METASOURCES = AUTO

noinst_HEADERS = kmdbentry.h kmfactory.h kmjob.h kmjobmanager.h kmmanager.h kmobject.h kmprinter.h \
		 kmuimanager.h kmvirtualmanager.h driver.h driveritem.h droptionview.h \
		 driverview.h matic.h kprintpreview.h kprinterimpl.h kprintdialog.h \
		 kprintdialogpage.h kpcopiespage.h kpdriverpage.h kprinterpropertydialog.h kpgeneralpage.h \
		 kpqtpage.h

include_HEADERS = kprinter.h

EXTRA_DIST = maticparser.yy maticscanner.ll kprintpreviewui.rc

#messages:
#        $(XGETTEXT) $(libkdeprint_la_SOURCES) -o $(podir)/kdeprint.pot

# installation of UI files
uidata_DATA = kprintpreviewui.rc
uidatadir = $(kde_confdir)/ui

# Test page
testpage_DATA = testprint.ps
testpagedir = $(kde_datadir)/kdeprint

# parser and scanner
parser: 
	bison -d -p matic $(srcdir)/maticparser.yy -o $(srcdir)/maticparser.cpp
	flex -omaticscanner.cpp -Pmatic $(srcdir)/maticscanner.ll

noinst_DATA = kdeprint

kdeprint:
	$(LN_S) $(srcdir)/management kdeprint
