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


INCLUDES = -I$(top_srcdir)/kdeui -I$(top_srcdir)/kdecore $(QT_INCLUDES) $(X_INCLUDES)
LDFLAGS = -L$(topdir)/kdecore/.libs -L$(topdir)/kdeui/.libs -rpath $(topdir)/kdecore/.libs -rpath $(topdir)/kdeui/.libs $(QT_LDFLAGS) $(X_LDFLAGS)

LDADD = $(LIB_KDEUI)

DISTCLEANFILES = $(BUILT_SOURCES)

check_PROGRAMS = kconfigtest kledlamptest kprogresstest \
		kcolordlgtest kdatepicktest kfontdialogtest	\
		kmsgboxtest kpopuptest ktabctltest ktreelisttest	\
		kseparatortest kiconloadertest		\
		kwindowtest klinetest kstatusbartest kspinboxtest	\
		kprocesstest kpannertest kbuttonboxtest ksimpleconfigtest \
		kcharsetstest kledtest kcontainertest klocaletest krulertest \
		knotebooktest kwizardtest

noinst_HEADERS = kdatepicktestdlg.h kmsgboxtest.h ktreelisttest.h \
		ktabctltest.h kiconloadertestmain.h	\
		kwindowtest.h klinetest.h kstatusbartest.h kspinboxtest.h \
		kprocesstest.h kcharsetstest.h kledtest.h kcontainertest.h \
		klocaletest.h krulertest.h

DISTCLEANFILES = kcharsetstest.moc.cpp kconfigtest.moc.cpp \
	kcontainertest.moc.cpp kdatepicktestdlg.moc.cpp \
	kiconloadertestmain.moc.cpp kledtest.moc.cpp \
	klinetest.moc.cpp klocaletest.moc.cpp kmsgboxtest.moc.cpp \
	kpannertest.moc.cpp kprocesstest.moc.cpp krulertest.moc.cpp \
	kspinboxtest.moc.cpp kstatusbartest.moc.cpp ktabctltest.moc.cpp \
	ktreelisttest.moc.cpp kwindowtest.moc.cpp 

kbuttonboxtest_SOURCES = kbuttonboxtest.cpp
kcharsetstest_SOURCES = kcharsetstest.cpp kcharsetstest.moc.cpp
kcolordlgtest_SOURCES = kcolordlgtest.cpp
kconfigtest_SOURCES = kconfigtest.cpp kconfigtest.moc.cpp
kcontainertest_SOURCES = kcontainertest.cpp kcontainertest.moc.cpp
kdatepicktest_SOURCES = kdatepicktest.cpp kdatepicktestdlg.cpp kdatepicktestdlg.moc.cpp
kfontdialogtest_SOURCES = kfontdialogtest.cpp
kiconloadertest_SOURCES = kiconloadertest.cpp kiconloadertestmain.cpp kiconloadertestmain.moc.cpp
kledlamptest_SOURCES = kledlamptest.cpp 
kledtest_SOURCES = kledtest.cpp kledtest.moc.cpp
klinetest_SOURCES = klinetest.cpp klinetest.moc.cpp
klocaletest_SOURCES = klocaletest.cpp klocaletest.moc.cpp
kmsgboxtest_SOURCES = kmsgboxtest.cpp kmsgboxtest.moc.cpp
knotebooktest_SOURCES = knotebooktest.cpp
kpannertest_SOURCES = kpannertest.cpp kpannertest.moc.cpp
kpopuptest_SOURCES = kpopuptest.cpp
kprocesstest_SOURCES = kprocesstest.cpp kprocesstest.moc.cpp
kprogresstest_SOURCES = kprogresstest.cpp
kquickhelptest_SOURCE = kquickhelptest.cpp
krulertest_SOURCES = krulertest.cpp krulertest.moc.cpp
kseparatortest_SOURCES = kseparatortest.cpp
ksimpleconfigtest_SOURCES = ksimpleconfigtest.cpp
kspinboxtest_SOURCES = kspinboxtest.cpp kspinboxtest.moc.cpp
kstatusbartest_SOURCES = kstatusbartest.cpp kstatusbartest.moc.cpp
ktabctltest_SOURCES = ktabctltest.cpp ktabctltest.moc.cpp
ktreelisttest_SOURCES = ktreelisttest.cpp ktreelisttest.moc.cpp
kwindowtest_SOURCES = kwindowtest.cpp kwindowtest.moc.cpp
kwizardtest_SOURCES = kwizardtest.cpp

kcharsetstest.moc.cpp: kcharsetstest.h
	$(MOC) $(srcdir)/kcharsetstest.h -o kcharsetstest.moc.cpp

kconfigtest.moc.cpp: kconfigtest.h
	$(MOC) $(srcdir)/kconfigtest.h -o kconfigtest.moc.cpp

kcontainertest.moc.cpp: kcontainertest.h
	$(MOC) $(srcdir)/kcontainertest.h -o kcontainertest.moc.cpp

kdatepicktestdlg.moc.cpp: kdatepicktestdlg.h
	$(MOC) $(srcdir)/kdatepicktestdlg.h -o kdatepicktestdlg.moc.cpp

kiconloadertestmain.moc.cpp: kiconloadertestmain.h
	$(MOC) $(srcdir)/kiconloadertestmain.h -o kiconloadertestmain.moc.cpp

kledtest.moc.cpp: kledtest.h
	$(MOC) $(srcdir)/kledtest.h -o kledtest.moc.cpp

klinetest.moc.cpp: klinetest.h
	$(MOC) $(srcdir)/klinetest.h -o klinetest.moc.cpp

klocaletest.moc.cpp: klocaletest.h
	$(MOC) $(srcdir)/klocaletest.h -o klocaletest.moc.cpp

kmsgboxtest.moc.cpp: kmsgboxtest.h
	$(MOC) $(srcdir)/kmsgboxtest.h -o kmsgboxtest.moc.cpp

kpannertest.moc.cpp: kpannertest.h
	$(MOC) $(srcdir)/kpannertest.h -o kpannertest.moc.cpp

kprocesstest.moc.cpp: kprocesstest.h
	$(MOC) $(srcdir)/kprocesstest.h -o kprocesstest.moc.cpp

krulertest.moc.cpp: krulertest.h
	$(MOC) $(srcdir)/krulertest.h -o krulertest.moc.cpp

kspinboxtest.moc.cpp: kspinboxtest.h
	$(MOC) $(srcdir)/kspinboxtest.h -o kspinboxtest.moc.cpp

kstatusbartest.moc.cpp: kstatusbartest.h
	$(MOC) $(srcdir)/kstatusbartest.h -o kstatusbartest.moc.cpp

ktabctltest.moc.cpp: ktabctltest.h
	$(MOC) $(srcdir)/ktabctltest.h -o ktabctltest.moc.cpp

ktreelisttest.moc.cpp: ktreelisttest.h
	$(MOC) $(srcdir)/ktreelisttest.h -o ktreelisttest.moc.cpp

kwindowtest.moc.cpp: kwindowtest.h
	$(MOC) $(srcdir)/kwindowtest.h -o kwindowtest.moc.cpp
