INCLUDES= -I$(top_srcdir)/kdefx -I$(top_srcdir)/kdeprint -I$(top_builddir)/kdeprint -I$(top_srcdir)/kio -I$(top_srcdir)/kfile -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkdeprint_management.la kdeinit_kaddprinterwizard.la
bin_PROGRAMS = kaddprinterwizard

kaddprinterwizard_SOURCES = dummy.cpp
kaddprinterwizard_LDADD = kdeinit_kaddprinterwizard.la
kaddprinterwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kdeinit_kaddprinterwizard_la_SOURCES = kaddprinterwizard.cpp
kdeinit_kaddprinterwizard_la_LIBADD = libkdeprint_management.la
kdeinit_kaddprinterwizard_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_RPATH)

libkdeprint_management_la_SOURCES = \
	kmdbcreator.cpp kmdriverdb.cpp kmdriverdbwidget.cpp \
	kmiconview.cpp kminfopage.cpp kmmainview.cpp \
	kmpages.cpp kmprinterview.cpp kmjobviewer.cpp \
	jobitem.cpp kmpropwidget.cpp kmpropcontainer.cpp kmpropertypage.cpp \
	kmpropgeneral.cpp cjanuswidget.cpp kmpropmembers.cpp kmpropbackend.cpp kmpropdriver.cpp \
	kmlistview.cpp kminstancepage.cpp kmtimer.cpp \
	kmwizard.cpp kmwizardpage.cpp kmwinfopage.cpp kmwpassword.cpp kmwsocketutil.cpp \
	kmwbackend.cpp kmwsocket.cpp kmwdriver.cpp kmwdriverselect.cpp kmwdrivertest.cpp \
	kmwinfobase.cpp kmwname.cpp kmwend.cpp kmwclass.cpp kmwlpd.cpp kmwfile.cpp kmdriverdialog.cpp \
	kmwsmb.cpp smbview.cpp kmconfigdialog.cpp kmconfigpage.cpp kmconfiggeneral.cpp kmspecialprinterdlg.cpp \
	kmwlocal.cpp kiconselectaction.cpp kmconfigpreview.cpp sidepixmap.cpp \
	pluginaction.cpp kxmlcommanddlg.cpp kxmlcommandselector.cpp kmconfigcommand.cpp \
	kmconfigfilter.cpp kmconfigfonts.cpp kmconfigjobs.cpp networkscanner.cpp
libkdeprint_management_la_LDFLAGS = $(all_libraries) -version-info 5:0:1 -no-undefined
libkdeprint_management_la_LIBADD = ../libkdeprint.la
libkdeprint_management_la_METASOURCES = AUTO

kdeprint_HEADERS = kmmainview.h kmjobviewer.h kmprinterpage.h kiconselectaction.h kmtimer.h
kdeprintdir = $(includedir)/kdeprint

noinst_HEADERS = kmdbcreator.h kmdriverdb.h kmdriverdbwidget.h kmiconview.h \
		 kminfopage.h kmmainview.h kmpages.h \
		 kmprinterpage.h kmprinterview.h kmjobviewer.h jobitem.h kmpropwidget.h kmpropcontainer.h \
		 kmpropertypage.h kmpropgeneral.h cjanuswidget.h kmpropmembers.h kmpropbackend.h kmpropdriver.h \
		 kmlistview.h kminstancepage.h kmwizard.h kmwizardpage.h kmwinfopage.h kmwpassword.h kmwsocketutil.h \
		 kmwbackend.h kmwsocket.h kmwdriver.h kmwdriverselect.h kmwdrivertest.h kmwinfobase.h kmwname.h kmwend.h \
		 kmwclass.h kmwlpd.h kmwfile.h kmdriverdialog.h kmwsmb.h smbview.h kmconfigdialog.h kmconfigpage.h \
		 kmconfiggeneral.h kmspecialprinterdlg.h kmwlocal.h kmconfigpreview.h sidepixmap.h \
		 pluginaction.h kxmlcommanddlg.h kxmlcommandselector.h kmconfigcommand.h \
		 kmconfigfilter.h kmconfigfonts.h kmconfigjobs.h networkscanner.h

dataicon_DATA = tileup.png tiledown.png side.png
dataicondir = $(kde_datadir)/kdeprint

dummy.cpp:
	echo > dummy.cpp

