
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


INCLUDES= $(all_includes)
EXTRA_DIST = $(data_DATA) CHANGES.kdatepicker USERS.kdatepicker

lib_LTLIBRARIES = libkdeui.la
libkdeui_la_LDFLAGS = -no-undefined $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 3:0
# we should get this to work when libtool 1.3 is released
libkdeui_la_LIBADD = ../kdecore/libkdecore.la

include_HEADERS = kledlamp.h kprogress.h kpanner.h kcolordlg.h kselect.h \
		kdatepik.h kdatetbl.h kfontdialog.h kmsgbox.h kpopmenu.h \
		ktabctl.h ktreelist.h kstatusbar.h ktopwidget.h \
		ktmainwindow.h ktoolbar.h kmenubar.h kbutton.h kslider.h \
		kseparator.h klined.h kcombo.h krestrictedline.h \
		kintegerline.h kspinbox.h kcolorbtn.h kiconloaderdialog.h \
		kwmmapp.h kbuttonbox.h ktablistbox.h kcontainer.h \
		knewpanner.h kcontrol.h keditcl.h ktoolboxmgr.h kled.h \
		kdbtn.h knotebook.h ktabbar.h kwizard.h kkeydialog.h \
		kurllabel.h kruler.h kquickhelp.h kcursor.h kaccelmenu.h \
		klineeditdlg.h kurlcompletion.h kwidgetid.h kthemebase.h \
		kthemestyle.h kcharselect.h kcolordrag.h kaction.h \
		kuiactions.h kintlineedit.h kactmenubuilder.h qxembed.h

libkdeui_la_SOURCES = kledlamp.cpp kprogress.cpp kpanner.cpp kcolordlg.cpp \
		kselect.cpp kdatepik.cpp kdatetbl.cpp kfontdialog.cpp \
		kmsgbox.cpp kpopmenu.cpp ktabctl.cpp ktreelist.cpp \
		kstatusbar.cpp ktopwidget.cpp ktmainwindow.cpp \
		kmenubar.cpp ktoolbar.cpp kbutton.cpp kslider.cpp \
		kseparator.cpp klined.cpp kcombo.cpp krestrictedline.cpp \
		kintegerline.cpp kspinbox.cpp kcontrol.cpp kcolorbtn.cpp \
		kiconloaderdialog.cpp kwmmapp.cpp kbuttonbox.cpp \
		ktablistbox.cpp knewpanner.cpp keditcl1.cpp keditcl2.cpp \
		ktoolboxmgr.cpp kled.cpp kdbtn.cpp knotebook.cpp \
		ktabbar.cpp kwizard.cpp kcontainer.cpp kkeydialog.cpp \
		kurllabel.cpp kruler.cpp kquickhelp.cpp kcursor.cpp \
		kaccelmenu.cpp klineeditdlg.cpp kurlcompletion.cpp \
		kthemebase.cpp kthemestyle.cpp kcharselect.cpp \
		kcolordrag.cpp kaction.cpp kuiactions.cpp \
		kactmenubuilder.cpp kintlineedit.cpp qxembed.cpp


libkdeui_la_METASOURCES = kbutton.moc kbuttonbox.moc kcolorbtn.moc \
	kcolordlg.moc kcombo.moc kcontainer.moc kcontrol.moc \
	kdatepik.moc kdatetbl.moc kdbtn.moc keditcl.moc kfontdialog.moc \
	kiconloaderdialog.moc kintegerline.moc kkeydialog.moc kled.moc \
	kledlamp.moc klined.moc kmenubar.moc kmsgbox.moc knewpanner.moc \
	knotebook.moc kpanner.moc kpopmenu.moc kprogress.moc \
	kquickhelp.moc krestrictedline.moc kruler.moc kselect.moc \
	kslider.moc kspinbox.moc kstatusbar.moc ktabbar.moc ktabctl.moc \
	ktablistbox.moc ktmainwindow.moc ktoolbar.moc ktoolboxmgr.moc \
	ktopwidget.moc ktreelist.moc kurllabel.moc kwizard.moc kwmmapp.moc \
	kaccelmenu.moc klineeditdlg.moc kurlcompletion.moc kthemebase.moc \
	kthemestyle.moc kcharselect.moc kcolordrag.moc kaction.moc \
	kuiactions.moc kintlineedit.moc qxembed.moc

data_DATA = error.xpm exclamation.xpm info.xpm stopsign.xpm question.xpm \
	arrow_up.xbm arrow_down.xbm arrow_left.xbm arrow_right.xbm	
datadir = $(kde_datadir)/kde/pics


