#    This file is part of the KDE libraries
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

INCLUDES = -I$(top_srcdir)/kdecore $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = kconfigtest klocaletest kprocesstest ksimpleconfigtest \
	kstddirstest kurltest kuniqueapptest ktempfiletest krandomsequencetest \
	kdebugtest ksocktest kstringhandlertest kcmdlineargstest kapptest \
	kmemtest kidlservertest kidlclienttest dcopkonqtest kipctest \
	cplusplustest kiconloadertest kresolvertest kmdcodectest knotifytest \
	ksortablevaluelisttest krfcdatetest testqtargs kprociotest kclipboardtest \
	kcharsetstest

noinst_HEADERS = kconfigtest.h klocaletest.h kprocesstest.h KIDLTest.h \
		kipctest.h kprociotest.h

METASOURCES = AUTO

LDADD = ../libkdecore.la
kconfigtest_SOURCES = kconfigtest.cpp
kdebugtest_SOURCES = kdebugtest.cpp
klocaletest_SOURCES = klocaletest.cpp
#klocaletest2_SOURCES = klocaletest2.cpp klocale.cpp libintl.cpp kcatalogue.cpp
#kcatalogue_SOURCES = kcatalogue.cpp libintl.cpp
ksimpleconfigtest_SOURCES = ksimpleconfigtest.cpp
kurltest_SOURCES = kurltest.cpp
kstddirstest_SOURCES = kstddirstest.cpp
kprocesstest_SOURCES = kprocesstest.cpp
kuniqueapptest_SOURCES = kuniqueapptest.cpp
kapptest_SOURCES = kapptest.cpp
ksocktest_SOURCES = ksocktest.cpp
ktempfiletest_SOURCES = ktempfiletest.cpp
krandomsequencetest_SOURCES = krandomsequencetest.cpp
kstringhandlertest_SOURCES = kstringhandlertest.cpp
kcmdlineargstest_SOURCES = kcmdlineargstest.cpp
dcopkonqtest_SOURCES = dcopkonqtest.cpp
kmemtest_SOURCES = kmemtest.cpp
kidlservertest_SOURCES = KIDLTest.cpp KIDLTest.skel
kidlclienttest_SOURCES = KIDLTestClient.cpp KIDLTest.stub
$(srcdir)/KIDLTestClient.cpp: KIDLTest_stub.h
kipctest_SOURCES = kipctest.cpp
cplusplustest_SOURCES = cplusplustest.cpp
kiconloadertest_SOURCES = kiconloadertest.cpp
kresolvertest_SOURCES = kresolvertest.cpp
kmdcodectest_SOURCES = kmdcodectest.cpp 
knotifytest_SOURCES = knotifytest.cpp
ksortablevaluelisttest_SOURCES = ksortablevaluelisttest.cpp
krfcdatetest_SOURCES = krfcdatetest.cpp
testqtargs_SOURCES = testqtargs.cpp
kprociotest_SOURCES = kprociotest.cpp
kclipboardtest_SOURCES = kclipboardtest.cpp
kcharsetstest_SOURCES = kcharsetstest.cpp
