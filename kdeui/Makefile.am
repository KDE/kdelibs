#	This file is part of the KDE libraries
#    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
#			  (C) 1997 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this library; see the file COPYING.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.


INCLUDES= $(QT_INCLUDES) $(X_INCLUDES) -I../kdecore
EXTRA_DIST = error.xpm exclamation.xpm info.xpm stopsign.xpm
DISTCLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES = libkdeui.la
libkdeui_la_LDFLAGS = -version-info 0:91  

include_HEADERS = kledlamp.h kprogress.h kpanner.h kcolordlg.h kselect.h \
		dither.h kdatepik.h kdatetbl.h \
		kfontdialog.h kmsgbox.h kpopmenu.h ktabctl.h\
		ktreelist.h kstatusbar.h ktopwidget.h ktmainwindow.h ktoolbar.h kmenubar.h \
		kbutton.h kslider.h kseparator.h \
		klined.h kcombo.h krestrictedline.h kintegerline.h	\
		kspinbox.h kcolorbtn.h kiconloaderdialog.h \
		kwmmapp.h kbuttonbox.h ktablistbox.h \
		knewpanner.h kcontrol.h keditcl.h

libkdeui_la_SOURCES = kledlamp.cpp kprogress.cpp kpanner.cpp kcolordlg.cpp\
		kselect.cpp dither.cpp kdatepik.cpp kdatetbl.cpp \
		kfontdialog.cpp kmsgbox.cpp kpopmenu.cpp ktabctl.cpp\
		ktreelist.cpp kstatusbar.cpp ktopwidget.cpp ktmainwindow.cpp kmenubar.cpp \
		ktoolbar.cpp kbutton.cpp kslider.cpp kseparator.cpp \
		klined.cpp kcombo.cpp krestrictedline.cpp \
		kintegerline.cpp kspinbox.cpp kcontrol.cpp \
		kcolorbtn.cpp kiconloaderdialog.cpp kwmmapp.cpp	\
		kbuttonbox.cpp ktablistbox.cpp knewpanner.cpp \
		keditcl1.cpp keditcl2.cpp

BUILT_SOURCES = kledlamp.moc kprogress.moc kpanner.moc kcolordlg.moc\
		kselect.moc ktabctl.moc kdatepik.moc \
		kmsgbox.moc kpopmenu.moc ktreelist.moc kdatetbl.moc\
		kfontdialog.moc ktoolbar.moc kstatusbar.moc kmenubar.moc \
		ktopwidget.moc ktmainwindow.moc kbutton.moc kslider.moc  \
		klined.moc kcombo.moc krestrictedline.moc kintegerline.moc \
		kspinbox.moc kcolorbtn.moc \
		kiconloaderdialog.moc kwmmapp.moc kbuttonbox.moc \
		ktablistbox.moc knewpanner.moc kcontrol.moc keditcl.moc 

kledlamp.cpp:kledlamp.moc
kledlamp.moc: kledlamp.h
	$(MOC) kledlamp.h -o kledlamp.moc

kcontrol.cpp:kcontrol.moc
kcontrol.moc: kcontrol.h
	$(MOC) kcontrol.h -o kcontrol.moc

kprogress.cpp:kprogress.moc
kprogress.moc: kprogress.h
	$(MOC) kprogress.h -o kprogress.moc

kpanner.cpp:kpanner.moc
kpanner.moc: kpanner.h
	$(MOC) kpanner.h -o kpanner.moc

kcolordlg.cpp:kcolordlg.moc
kcolordlg.moc: kcolordlg.h
	$(MOC) kcolordlg.h -o kcolordlg.moc

kselect.cpp:kselect.moc
kselect.moc: kselect.h
	$(MOC) kselect.h -o kselect.moc

ktabctl.cpp:ktabctl.moc
ktabctl.moc: ktabctl.h
	$(MOC) ktabctl.h -o ktabctl.moc

kdatepik.cpp:kdatepik.moc
kdatepik.moc: kdatepik.h
	$(MOC) kdatepik.h -o kdatepik.moc

kmsgbox.cpp:kmsgbox.moc
kmsgbox.moc: kmsgbox.h
	$(MOC) kmsgbox.h -o kmsgbox.moc

kpopmenu.cpp:kpopmenu.moc
kpopmenu.moc: kpopmenu.h
	$(MOC) kpopmenu.h -o kpopmenu.moc

ktreelist.cpp:ktreelist.moc
ktreelist.moc: ktreelist.h
	$(MOC) ktreelist.h -o ktreelist.moc

kdatetbl.cpp:kdatetbl.moc
kdatetbl.moc: kdatetbl.h
	$(MOC) kdatetbl.h -o kdatetbl.moc

kfontdialog.cpp:kfontdialog.moc
kfontdialog.moc: kfontdialog.h
	$(MOC) kfontdialog.h -o kfontdialog.moc


ktoolbar.cpp:ktoolbar.moc
ktoolbar.moc: ktoolbar.h
	$(MOC) ktoolbar.h -o ktoolbar.moc

kstatusbar.cpp:kstatusbar.moc
kstatusbar.moc: kstatusbar.h
	$(MOC) kstatusbar.h -o kstatusbar.moc

kmenubar.cpp:kmenubar.moc
kmenubar.moc: kmenubar.h
	$(MOC) kmenubar.h -o kmenubar.moc

ktopwidget.cpp:ktopwidget.moc
ktopwidget.moc: ktopwidget.h
	$(MOC) ktopwidget.h -o ktopwidget.moc

ktmainwindow.cpp:ktmainwindow.moc
ktmainwindow.moc: ktmainwindow.h
	$(MOC) ktmainwindow.h -o ktmainwindow.moc

kbutton.cpp:kbutton.moc
kbutton.moc: kbutton.h
	$(MOC) kbutton.h -o kbutton.moc

kslider.cpp:kslider.moc
kslider.moc: kslider.h
	$(MOC) kslider.h -o kslider.moc

klined.cpp:klined.moc
klined.moc: klined.h
	$(MOC) klined.h -o klined.moc

kcombo.cpp:kcombo.moc
kcombo.moc: kcombo.h
	$(MOC) kcombo.h -o kcombo.moc

kintegerline.cpp:kintegerline.moc
kintegerline.moc: kintegerline.h
	$(MOC) kintegerline.h -o kintegerline.moc

krestrictedline.cpp:krestrictedline.moc
krestrictedline.moc: krestrictedline.h
	$(MOC) krestrictedline.h -o krestrictedline.moc

kspinbox.cpp:kspinbox.moc
kspinbox.moc: kspinbox.h
	$(MOC) kspinbox.h -o kspinbox.moc

kcolorbtn.cpp:kcolorbtn.moc
kcolorbtn.moc: kcolorbtn.h
	$(MOC) kcolorbtn.h -o kcolorbtn.moc

kiconloaderdialog.cpp:kiconloaderdialog.moc
kiconloaderdialog.moc: kiconloaderdialog.h
	$(MOC) kiconloaderdialog.h -o kiconloaderdialog.moc

kwmmapp.cpp:kwmmapp.moc
kwmmapp.moc: kwmmapp.h
	$(MOC) kwmmapp.h -o kwmmapp.moc

kbuttonbox.cpp:kbuttonbox.moc
kbuttonbox.moc: kbuttonbox.h
	$(MOC) kbuttonbox.h -o kbuttonbox.moc

ktablistbox.cpp:ktablistbox.moc
ktablistbox.moc: ktablistbox.h
	$(MOC) ktablistbox.h -o ktablistbox.moc

keditcl1.cpp:keditcl.moc
keditcl.moc: keditcl.h
	$(MOC) keditcl.h -o keditcl.moc

knewpanner.cpp:knewpanner.moc
knewpanner.moc: knewpanner.h
	$(MOC) knewpanner.h -o knewpanner.moc

data_DATA = error.xpm exclamation.xpm info.xpm stopsign.xpm question.xpm
datadir = $(kde_datadir)/kde/pics

