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


INCLUDES = -I../kdeui -I../kdecore $(QT_INCLUDES) $(X_INCLUDES)

LDADD = -L$(topdir)/kdecore/.libs -lkdecore -L$(topdir)/kdeui/.libs -lkdeui $(QT_LDFLAGS) -lqt $(X_LDFLAGS) -lXext -lX11 $(LIBSOCKET)

DISTCLEANFILES = $(BUILT_SOURCES)

check_PROGRAMS = kconfigtest kledlamptest kprogresstest \
		kcolordlgtest kdatepicktest kfontdialogtest	\
		kmsgboxtest kpopuptest ktabctltest ktreelisttest	\
		kkeyconfigtest kseparatortest kiconloadertest		\
		kwindowtest klinetest kstatusbartest kspinboxtest	\
		kprocesstest kpannertest kbuttonboxtest ksimpleconfigtest \
		kcharsetstest kledtest kcontainertest

noinst_HEADERS = kdatepicktestdlg.h kmsgboxtest.h ktreelisttest.h \
		kkeyconfigtest.h ktabctltest.h kiconloadertestmain.h	\
		kwindowtest.h klinetest.h kstatusbartest.h kspinboxtest.h \
		kprocesstest.h kcharsetstest.h kledtest.h kcontainertest.h

BUILT_SOURCES = ktabctltest.moc kmsgboxtest.moc kdatepicktestdlg.moc \
		ktreelisttest.moc kkeyconfigtest.moc kiconloadertestmain.moc \
		kconfigtest.moc kpannertest.moc kwindowtest.moc klinetest.moc \
		kstatusbartest.moc kspinboxtest.moc kprocesstest.moc \
		kcharsetstest.moc kledtest.h kcontainertest.moc

ktabctltest.cpp: ktabctltest.moc
kiconloadertestmain.cpp: kiconloadertestmain.moc
kmsgboxtest.cpp: kmsgboxtest.moc
kdatepicktestdlg.cpp: kdatepicktestdlg.moc
ktreelisttest.cpp: ktreelisttest.moc
kkeyconfigtest.cpp: kkeyconfigtest.moc
kconfigtest.cpp: kconfigtest.moc
kpannertest.cpp: kpannertest.moc
kwindowtest.cpp: kwindowtest.moc
klinetest.cpp: klinetest.moc
kstatusbartest.cpp: kstatusbartest.moc
kspinboxtest.cpp: kspinboxtest.moc
kprocesstest.cpp: kprocesstest.moc
kcharsetstest.cpp: kcharsetstest.moc
kledtest.cpp: kledtest.moc
kcontainertest.cpp: kcontainertest.moc

kconfigtest_SOURCES = kconfigtest.cpp
kledlamptest_SOURCES = kledlamptest.cpp
kprogresstest_SOURCES = kprogresstest.cpp
kpannertest_SOURCES = kpannertest.cpp
kcolordlgtest_SOURCES = kcolordlgtest.cpp
kdatepicktest_SOURCES = kdatepicktest.cpp kdatepicktestdlg.cpp
kfontdialogtest_SOURCES = kfontdialogtest.cpp
kmsgboxtest_SOURCES = kmsgboxtest.cpp
kpopuptest_SOURCES = kpopuptest.cpp
ktabctltest_SOURCES = ktabctltest.cpp
ktreelisttest_SOURCES = ktreelisttest.cpp
kkeyconfigtest_SOURCES = kkeyconfigtest.cpp
kseparatortest_SOURCES = kseparatortest.cpp
kiconloadertest_SOURCES = kiconloadertest.cpp kiconloadertestmain.cpp
kwindowtest_SOURCES = kwindowtest.cpp
klinetest_SOURCES = klinetest.cpp
kstatusbartest_SOURCES = kstatusbartest.cpp
kspinboxtest_SOURCES = kspinboxtest.cpp
kprocesstest_SOURCES = kprocesstest.cpp
kbuttonboxtest_SOURCES = kbuttonboxtest.cpp
ksimpleconfigtest_SOURCES = ksimpleconfigtest.cpp
kcharsetstest_SOURCES = kcharsetstest.cpp
kledtest_SOURCES = kledtest.cpp
kcontainertest_SOURCES = kcontainertest.cpp

klinetest.moc: klinetest.h
	$(MOC) klinetest.h -o klinetest.moc

kspinboxtest.moc: kspinboxtest.h
	$(MOC) kspinboxtest.h -o kspinboxtest.moc

ktabctltest.moc: ktabctltest.h
	$(MOC) ktabctltest.h -o ktabctltest.moc

kmsgboxtest.moc: kmsgboxtest.h
	$(MOC) kmsgboxtest.h -o kmsgboxtest.moc

kdatepicktestdlg.moc: kdatepicktestdlg.h
	$(MOC) kdatepicktestdlg.h -o kdatepicktestdlg.moc

ktreelisttest.moc: ktreelisttest.h
	$(MOC) ktreelisttest.h -o ktreelisttest.moc

kwindowtest.moc: kwindowtest.h
	$(MOC) kwindowtest.h -o kwindowtest.moc

kpannertest.moc: kpannertest.h
	$(MOC) kpannertest.h -o kpannertest.moc

kkeyconfigtest.moc: kkeyconfigtest.h
	$(MOC) kkeyconfigtest.h -o kkeyconfigtest.moc

kiconloadertestmain.moc: kiconloadertestmain.h
	$(MOC) kiconloadertestmain.h -o kiconloadertestmain.moc

kstatusbartest.moc: kstatusbartest.h
	$(MOC) kstatusbartest.h -o kstatusbartest.moc

kconfigtest.moc: kconfigtest.h
	$(MOC) kconfigtest.h -o kconfigtest.moc

kprocesstest.moc: kprocesstest.h
	$(MOC) kprocesstest.h -o kprocesstest.moc

kcharsetstest.moc: kcharsetstest.h
	$(MOC) kcharsetstest.h -o kcharsetstest.moc

kledtest.moc: kledtest.h
	$(MOC) kledtest.h -o kledtest.moc

kcontainertest.moc: kcontainertest.h
	$(MOC) kcontainertest.h -o kcontainertest.moc
