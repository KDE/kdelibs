## KDE/src/lib/ui/test/Makefile.am
##
## (C) 1996 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@
INCLUDES= @QT_INCLUDES@ @X_INCLUDES@ -I../kdecore
EXTRA_DIST = error.xpm exclamation.xpm info.xpm stopsign.xpm
CLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES = libkdeui.la
libkdeui_la_LDFLAGS = -version-info 0:10  -rpath $(libdir)

include_HEADERS = kledlamp.h kprogress.h kpanner.h kcolordlg.h kselect.h \
		dither.h kdatepik.h kdatetbl.h \
		kfontdialog.h kmsgbox.h kpopmenu.h ktabctl.h\
		ktreelist.h kstatusbar.h ktopwidget.h ktoolbar.h kmenubar.h \
		kbutton.h kslider.h kseparator.h \
		klined.h kcombo.h krestrictedline.h kintegerline.h	\
		kspinbox.h kdocktoolbar.h kcolorbtn.h kiconloaderdialog.h

libkdeui_la_SOURCES = kledlamp.cpp kprogress.cpp kpanner.cpp kcolordlg.cpp\
		kselect.cpp dither.cpp kdatepik.cpp kdatetbl.cpp \
		kfontdialog.cpp kmsgbox.cpp kpopmenu.cpp ktabctl.cpp\
		ktreelist.cpp kstatusbar.cpp ktopwidget.cpp kmenubar.cpp \
		ktoolbar.cpp kbutton.cpp kslider.cpp kseparator.cpp \
		klined.cpp kcombo.cpp krestrictedline.cpp \
		kintegerline.cpp kspinbox.cpp kdocktoolbar.cpp \
		kcolorbtn.cpp kiconloaderdialog.cpp

BUILT_SOURCES = kledlamp.moc kprogress.moc kpanner.moc kcolordlg.moc\
		kselect.moc ktabctl.moc kdatepik.moc \
		kmsgbox.moc kpopmenu.moc ktreelist.moc kdatetbl.moc\
		kfontdialog.moc ktoolbar.moc kstatusbar.moc kmenubar.moc \
		ktopwidget.moc kbutton.moc kslider.moc  \
		klined.moc kcombo.moc krestrictedline.moc kintegerline.moc \
		kspinbox.moc kdocktoolbar.moc kcolorbtn.moc kiconloaderdialog.moc

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
klined.cpp: klined.moc
kcombo.cpp: kcombo.moc
kintegerline.cpp: kintegerline.moc
krestrictedline.cpp: krestrictedline.moc
kspinbox.cpp: kspinbox.moc
kdocktoolbar.cpp: kdocktoolbar.moc
kcolorbtn.cpp: kcolorbtn.moc
kiconloaderdialog.cpp: kiconloaderdialog.moc

kledlamp.moc: kledlamp.h
	$(MOC) kledlamp.h -o kledlamp.moc

kprogress.moc: kprogress.h
	$(MOC) kprogress.h -o kprogress.moc

kpanner.moc: kpanner.h
	$(MOC) kpanner.h -o kpanner.moc

kcolordlg.moc: kcolordlg.h
	$(MOC) kcolordlg.h -o kcolordlg.moc

kselect.moc: kselect.h
	$(MOC) kselect.h -o kselect.moc

ktabctl.moc: ktabctl.h
	$(MOC) ktabctl.h -o ktabctl.moc

kdatepik.moc: kdatepik.h
	$(MOC) kdatepik.h -o kdatepik.moc

kmsgbox.moc: kmsgbox.h
	$(MOC) kmsgbox.h -o kmsgbox.moc

kpopmenu.moc: kpopmenu.h
	$(MOC) kpopmenu.h -o kpopmenu.moc

ktreelist.moc: ktreelist.h
	$(MOC) ktreelist.h -o ktreelist.moc

kdatetbl.moc: kdatetbl.h
	$(MOC) kdatetbl.h -o kdatetbl.moc

kfontdialog.moc: kfontdialog.h
	$(MOC) kfontdialog.h -o kfontdialog.moc

ktoolbar.moc: ktoolbar.h
	$(MOC) ktoolbar.h -o ktoolbar.moc

kstatusbar.moc: kstatusbar.h
	$(MOC) kstatusbar.h -o kstatusbar.moc

kmenubar.moc: kmenubar.h
	$(MOC) kmenubar.h -o kmenubar.moc

ktopwidget.moc: ktopwidget.h
	$(MOC) ktopwidget.h -o ktopwidget.moc

kbutton.moc: kbutton.h
	$(MOC) kbutton.h -o kbutton.moc

kslider.moc: kslider.h
	$(MOC) kslider.h -o kslider.moc

klined.moc: klined.h
	$(MOC) klined.h -o klined.moc

kcombo.moc: kcombo.h
	$(MOC) kcombo.h -o kcombo.moc

kintegerline.moc: kintegerline.h
	$(MOC) kintegerline.h -o kintegerline.moc

krestrictedline.moc: krestrictedline.h
	$(MOC) krestrictedline.h -o krestrictedline.moc

kspinbox.moc: kspinbox.h
	$(MOC) kspinbox.h -o kspinbox.moc

kdocktoolbar.moc: kdocktoolbar.h
	$(MOC) kdocktoolbar.h -o kdocktoolbar.moc

kcolorbtn.moc: kcolorbtn.h
	$(MOC) kcolorbtn.h -o kcolorbtn.moc

kiconloaderdialog.moc: kiconloaderdialog.h
	$(MOC) kiconloaderdialog.h -o kiconloaderdialog.moc

data_DATA = error.xpm exclamation.xpm info.xpm stopsign.xpm
datadir = $(prefix)/share/apps/kde/pics

