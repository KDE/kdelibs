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
          -I$(top_srcdir)/kformula -I$(top_srcdir)/kio\
	  -I$(top_srcdir)/kfile $(all_includes)

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = kconfigtest kledlamptest kprogresstest \
		kcolordlgtest kdatepicktest kfontdialogtest	\
		kpopuptest ktabctltest kseparatortest kiconloadertest \
		kwindowtest kstatusbartest kprocesstest kbuttonboxtest \
	    ksimpleconfigtest kledtest kcontainertest klocaletest krulertest \
		kwizardtest kurltest kformulatest \
		kcharselecttest kstddirstest khtmltest \
		kopenwithtest kaboutdialogtest dialogbasetest \
		kgradienttest \
		kunbalancedgrdtest kcolortest knuminputtest kblendtest \
		kdualcolortest kpalettetest kmessageboxtest khashtest ktartest

noinst_HEADERS = \
		ktabctltest.h kiconloadertestmain.h	\
		kwindowtest.h kstatusbartest.h \
		kprocesstest.h kledtest.h kcontainertest.h \
		klocaletest.h krulertest.h kconfigtest.h kgradienttest.h \
		kcolortest.h kunbalancedgrdtest.h kblendtest.h \
		kdualcolortest.h khashtest.h

METASOURCES = kconfigtest.moc \
	kcontainertest.moc \
	kiconloadertestmain.moc kledtest.moc \
	klocaletest.moc knuminputtest.moc \
	kprocesstest.moc krulertest.moc \
	kstatusbartest.moc ktabctltest.moc \
	kwindowtest.moc kunbalancedgrdtest.moc kdualcolortest.moc \
	khashtest.moc

LDADD =  $(LIB_KDEUI)

kbuttonboxtest_SOURCES = kbuttonboxtest.cpp
kaboutdialogtest_SOURCES = kaboutdialogtest.cpp
kaboutdialogtest_LDADD = $(LIB_KIMGIO) $(LIB_KDEUI)
dialogbasetest_SOURCES = dialogbasetest.cpp
kcolordlgtest_SOURCES = kcolordlgtest.cpp
kconfigtest_SOURCES = kconfigtest.cpp
kconfigtest_LDADD = $(LIB_KDECORE)
kcontainertest_SOURCES = kcontainertest.cpp
kdatepicktest_SOURCES = kdatepicktest.cpp
kfontdialogtest_SOURCES = kfontdialogtest.cpp
kformulatest_SOURCES = kformulatest.cpp
kformulatest_LDADD = $(LIB_KFORMULA)
kiconloadertest_SOURCES = kiconloadertest.cpp kiconloadertestmain.cpp
kledlamptest_SOURCES = kledlamptest.cpp
kledtest_SOURCES = kledtest.cpp
klocaletest_SOURCES = klocaletest.cpp
klocaletest_LDADD = $(LIB_KDECORE)
kmessageboxtest_SOURCES = kmessageboxtest.cpp
knuminputtest_SOURCES = knuminputtest.cpp
kopenwithtest_SOURCES = kopenwithtest.cpp
kopenwithtest_LDADD = $(LIB_KIO)
kpopuptest_SOURCES = kpopuptest.cpp
kprocesstest_SOURCES = kprocesstest.cpp
kprogresstest_SOURCES = kprogresstest.cpp
krulertest_SOURCES = krulertest.cpp
kseparatortest_SOURCES = kseparatortest.cpp
ksimpleconfigtest_SOURCES = ksimpleconfigtest.cpp
kstatusbartest_SOURCES = kstatusbartest.cpp
ktabctltest_SOURCES = ktabctltest.cpp
kurltest_SOURCES = kurltest.cpp
kwindowtest_SOURCES = kwindowtest.cpp
kwizardtest_SOURCES = kwizardtest.cpp
kcharselecttest_SOURCES = kcharselecttest.cpp
kstddirstest_SOURCES = kstddirstest.cpp
kstddirstest_LDADD = $(LIB_KDECORE)
khtmltest_SOURCES = khtmltest.cpp
khtmltest_LDADD = $(LIB_KHTML)
kcolortest_SOURCES = kcolortest.cpp
kgradienttest_SOURCES = kgradienttest.cpp
khashtest_SOURCES = khashtest.cpp
kunbalancedgrdtest_SOURCES = kunbalancedgrdtest.cpp
kblendtest_SOURCES = kblendtest.cpp
kdualcolortest_SOURCES = kdualcolortest.cpp
kpalettetest_SOURCES = kpalettetest.cpp
ktartest_SOURCES = ktartest.cpp
ktartest_LDADD = $(LIB_KIO)

