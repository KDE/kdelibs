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
libkdeui_la_LDFLAGS = -version-info 0:8  -rpath $(libdir)

include_HEADERS = kledlamp.h kprogress.h kpanner.h kcolordlg.h kselect.h \
		dither.h kdatepik.h kdatetbl.h kedit.h keditconstants.h \
		kfontdialog.h kmsgbox.h kpopmenu.h ktabctl.h\
<<<<<<< Makefile.am
		ktreelist.h kstatusbar.h ktopwidget.h ktoolbar.h	\
		kbutton.h kslider.h kseparator.h kdocktoolbar.h		\
		kiconloader.h
=======
		ktreelist.h kstatusbar.h ktopwidget.h ktoolbar.h kmenubar.h \
		kbutton.h kslider.h kseparator.h
>>>>>>> 1.5

libkdeui_la_SOURCES = kledlamp.cpp kprogress.cpp kpanner.cpp kcolordlg.cpp\
		kselect.cpp dither.cpp kdatepik.cpp kdatetbl.cpp kedit.cpp \
		kfontdialog.cpp kmsgbox.cpp kpopmenu.cpp ktabctl.cpp\
<<<<<<< Makefile.am
		ktreelist.cpp kstatusbar.cpp ktopwidget.cpp\
		ktoolbar.cpp kbutton.cpp kslider.cpp kseparator.cpp	\
		kdocktoolbar.cpp kiconloader.cpp
=======
		ktreelist.cpp kstatusbar.cpp ktopwidget.cpp kmenubar.cpp \
		ktoolbar.cpp kbutton.cpp kslider.cpp kseparator.cpp
>>>>>>> 1.5

BUILT_SOURCES = kledlamp.moc kprogress.moc kpanner.moc kcolordlg.moc\
		kselect.moc ktabctl.moc kdatepik.moc kedit.moc\
		kmsgbox.moc kpopmenu.moc ktreelist.moc kdatetbl.moc\
<<<<<<< Makefile.am
		kfontdialog.moc ktoolbar.moc kstatusbar.moc \
		ktopwidget.moc kbutton.moc kslider.moc kdocktoolbar.moc	\
		kiconloader.moc
=======
		kfontdialog.moc ktoolbar.moc kstatusbar.moc kmenubar.cpp \
		ktopwidget.moc kbutton.moc kslider.moc
>>>>>>> 1.5

data_DATA = error.xpm exclamation.xpm info.xpm stopsign.xpm

%.moc: %.h
	$(MOC) $< -o $@

