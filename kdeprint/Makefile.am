
if include_cups_SUBDIR
CUPSSUBDIR=cups
endif

SUBDIRS = . management lpdunix $(CUPSSUBDIR) lpr rlpr ext filters pics tools tests

INCLUDES= -I$(top_srcdir)/kdefx -I$(top_srcdir)/kded -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kded_kdeprintd.la

kded_kdeprintd_la_SOURCES = kdeprintd.skel kdeprintd.cpp kprintprocess.cpp
kded_kdeprintd_la_LIBADD = $(LIB_KIO)
kded_kdeprintd_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kded_kdeprintd_la_METASOURCES = AUTO

lib_LTLIBRARIES = libkdeprint.la

libkdeprint_la_SOURCES = kmfactory.cpp kmjob.cpp kmjobmanager.cpp kmmanager.cpp kmprinter.cpp \
		         kmuimanager.cpp kmvirtualmanager.cpp driver.cpp driveritem.cpp droptionview.cpp \
			 driverview.cpp maticscanner.cpp maticparser.cpp matic.cpp kmdbentry.cpp \
		         kprintpreview.cpp kprintdialog.cpp kprinter.cpp kprinterimpl.cpp kprintdialogpage.cpp \
			 kpcopiespage.cpp kpdriverpage.cpp kprinterpropertydialog.cpp kpgeneralpage.cpp kpqtpage.cpp \
			 kmthreadjob.cpp kpipeprocess.cpp kpfilterpage.cpp \
			 kmspecialmanager.cpp kdeprintcheck.cpp plugincombobox.cpp kpreloadobject.cpp kfilelist.cpp \
			 kprintaction.cpp kpfileselectpage.cpp treecombobox.cpp kmfactory.skel kxmlcommand.cpp \
			 util.cpp printerfilter.cpp marginpreview.cpp marginvaluewidget.cpp marginwidget.cpp \
			 kpmarginpage.cpp messagewindow.cpp
libkdeprint_la_LDFLAGS = $(all_libraries) -version-info 5:0:1 -no-undefined
libkdeprint_la_LIBADD = $(LIB_KPARTS) $(LIBZ)
libkdeprint_la_DEPENDENCIES =  $(LIB_KPARTS) 
libkdeprint_la_METASOURCES = AUTO

noinst_HEADERS = kmdbentry.h kmfactory.h \
		 kmuimanager.h kmvirtualmanager.h driveritem.h droptionview.h \
		 driverview.h matic.h kprintpreview.h kprinterimpl.h kprintdialog.h \
		 kpcopiespage.h kpdriverpage.h kprinterpropertydialog.h kpgeneralpage.h \
		 kpqtpage.h kprintprocess.h kmthreadjob.h kpipeprocess.h kdeprintd.h \
		 kpfilterpage.h kmspecialmanager.h kdeprintcheck.h plugincombobox.h \
		 kfilelist.h kpfileselectpage.h treecombobox.h kxmlcommand.h \
		 util.h driverparse.h printerfilter.h marginpreview.h marginwidget.h \
		 marginvaluewidget.h kpmarginpage.h messagewindow.h

include_HEADERS = kprinter.h kprintaction.h

kdeprint_HEADERS = kprintdialogpage.h kpreloadobject.h kmmanager.h kmprinter.h kmobject.h kprintdialog.h \
		   driver.h kmjobmanager.h kmjob.h
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

# Config file
configdir = $(kde_confdir)
config_DATA = kdeprintrc

# parser and scanner
parser:
	bison -d -p matic $(srcdir)/maticparser.yy -o $(srcdir)/maticparser.cpp
	flex -omaticscanner.cpp -Pmatic $(srcdir)/maticscanner.ll
	echo "#ifndef maticparser_cpp_h" > $(srcdir)/maticparser.cpp.hh
	echo "#define maticparser_cpp_h" >> $(srcdir)/maticparser.cpp.hh
	cat $(srcdir)/maticparser.cpp.h >> $(srcdir)/maticparser.cpp.hh
	echo "#endif" >> $(srcdir)/maticparser.cpp.hh
	mv $(srcdir)/maticparser.cpp.hh $(srcdir)/maticparser.cpp.h

noinst_DATA = kdeprint

kdeprint:
	$(LN_S) $(srcdir)/management kdeprint

# convenience static library
noinst_LIBRARIES = libdriverparse.a
libdriverparse_a_SOURCES = driverparse.c
