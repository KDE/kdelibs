#	This file is part of the KDE libraries
#    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
#              (C) 1997-1998 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

top_realbuilddir = `cd $(top_builddir) && pwd`


INCLUDES = -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui \
	  $(all_includes)

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

# Please keep sorted
check_PROGRAMS = kaboutdialogtest kblendtest kbuttonboxtest kcharselecttest \
	kcolortest kcolordlgtest \
	kcontainertest kdatepicktest kdialogbasetest \
	kdualcolortest kfontdialogtest \
	knewgradienttest kgradienttest khashtest \
	kiconloadertest kledtest klineeditdlgtest \
	kmessageboxtest knuminputtest \
	kpalettetest \
	kpopuptest kprogresstest krulertest kseparatortest \
	kstatusbartest ktabctltest \
	kunbalancedgrdtest kwindowtest kwizardtest

noinst_HEADERS = \
		ktabctltest.h kiconloadertestmain.h	\
		kwindowtest.h kstatusbartest.h \
		kledtest.h kcontainertest.h \
		krulertest.h knewgradienttest.h kgradienttest.h \
		kcolortest.h kunbalancedgrdtest.h kblendtest.h \
		kdualcolortest.h khashtest.h knuminputtest.h

METASOURCES = AUTO

LDADD =  $(LIB_KDEUI)

# Please keep sorted
kaboutdialogtest_SOURCES = kaboutdialogtest.cpp
kblendtest_SOURCES = kblendtest.cpp
kbuttonboxtest_SOURCES = kbuttonboxtest.cpp
kcharselecttest_SOURCES = kcharselecttest.cpp
kcolordlgtest_SOURCES = kcolordlgtest.cpp
kcolortest_SOURCES = kcolortest.cpp
kcontainertest_SOURCES = kcontainertest.cpp
kdatepicktest_SOURCES = kdatepicktest.cpp
kdialogbasetest_SOURCES = kdialogbasetest.cpp
kdualcolortest_SOURCES = kdualcolortest.cpp
kfontdialogtest_SOURCES = kfontdialogtest.cpp
kgradienttest_SOURCES = kgradienttest.cpp
knewgradienttest_SOURCES = knewgradienttest.cpp
khashtest_SOURCES = khashtest.cpp
kiconloadertest_SOURCES = kiconloadertest.cpp kiconloadertestmain.cpp
kledtest_SOURCES = kledtest.cpp
klineeditdlgtest_SOURCES = klineeditdlgtest.cpp
kmessageboxtest_SOURCES = kmessageboxtest.cpp
knuminputtest_SOURCES = knuminputtest.cpp
kpalettetest_SOURCES = kpalettetest.cpp
kpopuptest_SOURCES = kpopuptest.cpp
kprogresstest_SOURCES = kprogresstest.cpp
krulertest_SOURCES = krulertest.cpp
kseparatortest_SOURCES = kseparatortest.cpp
kstatusbartest_SOURCES = kstatusbartest.cpp
ktabctltest_SOURCES = ktabctltest.cpp
kunbalancedgrdtest_SOURCES = kunbalancedgrdtest.cpp
kwindowtest_SOURCES = kwindowtest.cpp
kwizardtest_SOURCES = kwizardtest.cpp
