## KDE/src/lib/ui/test/Makefile.am
##
## (C) 1996 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@
INCLUDES= @QT_INCLUDES@ @X_INCLUDES@ -I../kdecore
EXTRA_DIST = error.xpm exclamation.xpm info.xpm stopsign.xpm
CLEANFILES = *.moc
SUFFIXES = .moc

lib_LTLIBRARIES = libkdeui.la
libkdeui_la_LDFLAGS = -version-info 0:10  -rpath $(libdir)

include_HEADERS = kledlamp.h kprogress.h kpanner.h kcolordlg.h kselect.h \
		dither.h kdatepik.h kdatetbl.h \
		kfontdialog.h kmsgbox.h kpopmenu.h ktabctl.h\
		ktreelist.h kstatusbar.h ktopwidget.h ktoolbar.h kmenubar.h \
		kbutton.h kslider.h kseparator.h kiconloader.h	\
		klined.h kcombo.h

libkdeui_la_SOURCES = kledlamp.cpp kprogress.cpp kpanner.cpp kcolordlg.cpp\
		kselect.cpp dither.cpp kdatepik.cpp kdatetbl.cpp \
		kfontdialog.cpp kmsgbox.cpp kpopmenu.cpp ktabctl.cpp\
		ktreelist.cpp kstatusbar.cpp ktopwidget.cpp kmenubar.cpp \
		ktoolbar.cpp kbutton.cpp kslider.cpp kseparator.cpp	\
		kiconloader.cpp klined.cpp kcombo.cpp	

BUILT_SOURCES = kledlamp.moc kprogress.moc kpanner.moc kcolordlg.moc\
		kselect.moc ktabctl.moc kdatepik.moc \
		kmsgbox.moc kpopmenu.moc ktreelist.moc kdatetbl.moc\
		kfontdialog.moc ktoolbar.moc kstatusbar.moc kmenubar.moc \
		ktopwidget.moc kbutton.moc kslider.moc kiconloader.moc	\
		klined.moc kcombo.moc

kledlamp.cpp: kledlamp.moc
kprogress.cpp: kprogress.moc
kpanner.cpp: kpanner.moc
kcolordlg.cpp: kcolordlg.moc
kselect.cpp: kselect.moc
ktabctl.cpp: ktabctl.moc
kdatepik.cpp: kdatepik.moc
kmsgbox.cpp: kmsgbox.moc
kpopmenu.cpp: kpopmenu.moc
ktreelist.cpp: ktreelist.moc
kdatetbl.cpp: kdatetbl.moc
kfontdialog.cpp: kfontdialog.moc
ktoolbar.cpp: ktoolbar.moc
kstatusbar.cpp: kstatusbar.moc
kmenubar.cpp: kmenubar.moc
ktopwidget.cpp: ktopwidget.moc
kbutton.cpp: kbutton.moc
kslider.cpp: kslider.moc
kiconloader.cpp: kiconloader.moc
klined.cpp: klined.moc
kcombo.cpp: kcombo.moc

data_DATA = error.xpm exclamation.xpm info.xpm stopsign.xpm

%.moc: %.h
	$(MOC) $< -o $@

