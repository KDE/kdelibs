
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

SUBDIRS = . tests

INCLUDES= $(all_includes)

EXTRA_DIST = CHANGES.kdatepicker USERS.kdatepicker

#uncomment that if the menus look strange
AM_CPPFLAGS = -DQT2_WORKAROUND

lib_LTLIBRARIES = libkdeui.la
libkdeui_la_LDFLAGS = -no-undefined $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 3:0
# we should get this to work when libtool 1.3 is released
libkdeui_la_LIBADD = ../kdecore/libkdecore.la

include_HEADERS = kprogress.h kcolordlg.h kselect.h \
		kdatepik.h kdatetbl.h kfontdialog.h kpopmenu.h \
		ktabctl.h kstatusbar.h \
		ktmainwindow.h ktmlayout.h ktoolbar.h kmenubar.h \
		knuminput.h kseparator.h klined.h \
		krestrictedline.h kcolorbtn.h kiconloaderdialog.h \
		kapplet.h kwmmapp.h kbuttonbox.h ktablistbox.h \
		kcontainer.h kcontrol.h keditcl.h ktoolboxmgr.h kled.h \
		kdbtn.h ktabbar.h kwizard.h kkeydialog.h \
		kurllabel.h kruler.h kcursor.h kaccelmenu.h \
		klineeditdlg.h kurlcompletion.h kwidgetid.h kthemebase.h \
		kthemestyle.h kcharselect.h kcolordrag.h qxembed.h \
		knumvalidator.h kdialog.h kdialogbase.h \
		kjanuswidget.h kaboutdialog.h kpixmapeffect.h \
		kauthicon.h kmessagebox.h \
		kdualcolorbtn.h ktoolbarbutton.h \
		ktoolbarradiogroup.h ktextbrowser.h kaction.h khelpmenu.h

# the order of the sources isn't randomly (at least not completly).
# the order is predictated by the use of X11 header files
libkdeui_la_SOURCES = kthemestyle.cpp kurllabel.cpp knuminput.cpp  \
		kiconloaderdialog.cpp ktablistbox.cpp ktoolbar.cpp \
		klined.cpp kprogress.cpp \
		kcolordlg.cpp kselect.cpp kdatepik.cpp kdatetbl.cpp \
		kfontdialog.cpp kpopmenu.cpp ktabctl.cpp \
		kstatusbar.cpp ktmainwindow.cpp \
		ktmlayout.cpp kmenubar.cpp \
		kseparator.cpp krestrictedline.cpp  \
		kcontrol.cpp kcolorbtn.cpp kapplet.cpp kwmmapp.cpp \
		kbuttonbox.cpp keditcl1.cpp keditcl2.cpp ktoolboxmgr.cpp \
		kled.cpp kdbtn.cpp ktabbar.cpp kwizard.cpp kcontainer.cpp \
		kkeydialog.cpp kruler.cpp kcursor.cpp kaccelmenu.cpp \
		klineeditdlg.cpp kurlcompletion.cpp kthemebase.cpp \
		kcharselect.cpp kcolordrag.cpp qxembed.cpp \
		knumvalidator.cpp kdialog.cpp kdialogbase.cpp \
		kjanuswidget.cpp kaboutdialog.cpp kpixmapeffect.cpp \
		kauthicon.cpp kmessagebox.cpp kdualcolorbtn.cpp \
		ktoolbarradiogroup.cpp ktoolbarbutton.cpp ktoolbaritem.cpp \
		ktextbrowser.cpp kaction.cpp khelpmenu.cpp

libkdeui_la_METASOURCES = AUTO

