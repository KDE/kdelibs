## KDE/src/lib/ui/test/Makefile.am
##
## (C) 1996 Matthias Kalle Dalheimer
## (C) 1997 Stephan Kulow

MOC = @MOC@
INCLUDES= -I@qt_includes@ -I@x_includes@ -I../kdecore
EXTRA_DIST = error.xpm exclamation.xpm info.xpm stopsign.xpm
CLEANFILES = *.moc
SUFFIXES = .moc

lib_LTLIBRARIES = libkdeui.la
libkdeui_la_LDFLAGS = -version-info 0:7  -rpath $(libdir)

include_HEADERS = kledlamp.h kprogress.h kpanner.h kcolordlg.h kselect.h \
		dither.h kdatepik.h kdatetbl.h kedit.h keditconstants.h \
		kfontdialog.h kmsgbox.h kpopmenu.h ktabctl.h\
		ktreelist.h


libkdeui_la_SOURCES = kledlamp.cpp kprogress.cpp kpanner.cpp kcolordlg.cpp\
		kselect.cpp dither.cpp kdatepik.cpp kdatetbl.cpp kedit.cpp \
		kfontdialog.cpp kmsgbox.cpp kpopmenu.cpp ktabctl.cpp\
		ktreelist.cpp

BUILT_SOURCES = kledlamp.moc kprogress.moc kpanner.moc kcolordlg.moc\
		kselect.moc ktabctl.moc kdatepik.moc kedit.moc\
		kmsgbox.moc kpopmenu.moc ktreelist.moc kdatetbl.moc\
		kfontdialog.moc

data_DATA = error.xpm exclamation.xpm info.xpm stopsign.xpm

%.moc: %.h
	$(MOC) $< -o $@

