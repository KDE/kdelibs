
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
		ktmainwindow.h ktmlayout.h ktoolbar.h kmenubar.h \
		kbutton.h kslider.h \
		kseparator.h klined.h kcombo.h krestrictedline.h \
		kcolorbtn.h kiconloaderdialog.h \
		kwmmapp.h kbuttonbox.h ktablistbox.h kcontainer.h \
		knewpanner.h kcontrol.h keditcl.h ktoolboxmgr.h kled.h \
		kdbtn.h ktabbar.h kwizard.h kkeydialog.h \
		kurllabel.h kruler.h kquickhelp.h kcursor.h kaccelmenu.h \
		klineeditdlg.h kurlcompletion.h kwidgetid.h kthemebase.h \
		kthemestyle.h kcharselect.h kcolordrag.h \
		qxembed.h knumvalidator.h knumlineedit.h \
		kdialog.h \
		dialogbase.h kaboutdialog.h

# the order of the sources isn't randomly (at least not completly).
# the order is predictated by the use of X11 header files
libkdeui_la_SOURCES = kthemestyle.cpp kurllabel.cpp  \
		kiconloaderdialog.cpp ktablistbox.cpp ktoolbar.cpp \
		klined.cpp kledlamp.cpp kprogress.cpp kpanner.cpp \
		kcolordlg.cpp kselect.cpp kdatepik.cpp kdatetbl.cpp \
		kfontdialog.cpp kmsgbox.cpp kpopmenu.cpp ktabctl.cpp \
		ktreelist.cpp kstatusbar.cpp ktopwidget.cpp ktmainwindow.cpp \
		ktmlayout.cpp kmenubar.cpp kbutton.cpp kslider.cpp \
		kseparator.cpp kcombo.cpp krestrictedline.cpp \
		kcontrol.cpp kcolorbtn.cpp kwmmapp.cpp kbuttonbox.cpp\
		knewpanner.cpp keditcl1.cpp keditcl2.cpp \
		ktoolboxmgr.cpp kled.cpp kdbtn.cpp \
		ktabbar.cpp kwizard.cpp kcontainer.cpp kkeydialog.cpp \
		kruler.cpp kquickhelp.cpp kcursor.cpp \
		kaccelmenu.cpp klineeditdlg.cpp kurlcompletion.cpp \
		kthemebase.cpp kcharselect.cpp \
		kcolordrag.cpp qxembed.cpp \
		knumvalidator.cpp knumlineedit.cpp kdialog.cpp \
		dialogbase.cpp kaboutdialog.cpp

libkdeui_la_METASOURCES = kbutton.moc kbuttonbox.moc kcolorbtn.moc \
	kcolordlg.moc kcombo.moc kcontainer.moc kcontrol.moc \
	kdatepik.moc kdatetbl.moc kdbtn.moc keditcl.moc kfontdialog.moc \
	kiconloaderdialog.moc kkeydialog.moc kled.moc \
	kledlamp.moc klined.moc kmenubar.moc kmsgbox.moc knewpanner.moc \
	kpanner.moc kpopmenu.moc kprogress.moc \
	kquickhelp.moc krestrictedline.moc kruler.moc kselect.moc \
	kslider.moc kstatusbar.moc ktabbar.moc ktabctl.moc \
	ktablistbox.moc ktmainwindow.moc ktoolbar.moc ktoolboxmgr.moc \
	ktopwidget.moc ktreelist.moc kurllabel.moc kwizard.moc kwmmapp.moc \
	kaccelmenu.moc klineeditdlg.moc kurlcompletion.moc kthemebase.moc \
	kthemestyle.moc kcharselect.moc kcolordrag.moc \
	qxembed.moc knumlineedit.moc kdialog.moc \
	dialogbase.moc kaboutdialog.moc

data_DATA = error.xpm exclamation.xpm info.xpm stopsign.xpm question.xpm \
	arrow_up.xbm arrow_down.xbm arrow_left.xbm arrow_right.xbm	
datadir = $(kde_datadir)/kde/pics


