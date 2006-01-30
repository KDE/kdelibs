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
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

INCLUDES = -I$(top_srcdir)/kdecore $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = kconfigtestgui klocaletest kprocesstest ksimpleconfigtest \
	kstddirstest kurltest kurltest4 kuniqueapptest krandomsequencetest \
	kdebugtest kstringhandlertest kcmdlineargstest kapptest \
	kmemtest kidlservertest kidlclienttest dcopkonqtest kipctest \
	cplusplustest kiconloadertest kmdcodectest knotifytest kdatetimetest \
	ksortablevaluelisttest krfcdatetest testqtargs kprociotest \
	kcharsetstest kcalendartest kmacroexpandertest kshelltest \
	kxerrorhandlertest startserviceby kglobaltest ktimezonestest \
	kconfigtest ktempfiletest kurlmimetest kstdacceltest klibloadertest \
	kcompletiontest klockfiletest ktempdirtest ksharedptrtest kfiltertest \
	kdatetimetest ksavefiletest

# TODO kurltest4 when it works

# List of tests to run automatically from "make check"
TESTS = kurltest kcompletiontest\
	kconfigtest ktempfiletest kurlmimetest kstdacceltest \
	kstringhandlertest krfcdatetest kcalendartest klibloadertest

# Test module for klibloadertest. Ideally we shouldn't install it.
kde_module_LTLIBRARIES = libklibloadertestmodule.la
libklibloadertestmodule_la_SOURCES = klibloadertest_module.cpp
klibloadertest: libklibloadertestmodule.la

METASOURCES = AUTO

LDADD = ../libkdecore.la -lQtTest_debug

# Interactive test programs
kconfigtestgui_SOURCES = kconfigtestgui.cpp
kdebugtest_SOURCES = kdebugtest.cpp
klocaletest_SOURCES = klocaletest.cpp
#klocaletest2_SOURCES = klocaletest2.cpp klocale.cpp libintl.cpp kcatalog.cpp
#kcatalog_SOURCES = kcatalog.cpp libintl.cpp
ksimpleconfigtest_SOURCES = ksimpleconfigtest.cpp
kurltest_SOURCES = kurltest.cpp
kstddirstest_SOURCES = kstddirstest.cpp
kprocesstest_SOURCES = kprocesstest.cpp
kuniqueapptest_SOURCES = kuniqueapptest.cpp
kapptest_SOURCES = kapptest.cpp
krandomsequencetest_SOURCES = krandomsequencetest.cpp
kcmdlineargstest_SOURCES = kcmdlineargstest.cpp
dcopkonqtest_SOURCES = dcopkonqtest.cpp
kmemtest_SOURCES = kmemtest.cpp
kidlservertest_SOURCES = KIDLTest.cpp KIDLTest.skel
kidlclienttest_SOURCES = KIDLTestClient.cpp KIDLTest.stub
$(srcdir)/KIDLTestClient.cpp: KIDLTest_stub.h
kipctest_SOURCES = kipctest.cpp
cplusplustest_SOURCES = cplusplustest.cpp
kiconloadertest_SOURCES = kiconloadertest.cpp
kmdcodectest_SOURCES = kmdcodectest.cpp
knotifytest_SOURCES = knotifytest.cpp
ksortablevaluelisttest_SOURCES = ksortablevaluelisttest.cpp
testqtargs_SOURCES = testqtargs.cpp
kprociotest_SOURCES = kprociotest.cpp
kcharsetstest_SOURCES = kcharsetstest.cpp
kmacroexpandertest_SOURCES = kmacroexpandertest.cpp
kshelltest_SOURCES = kshelltest.cpp
kxerrorhandlertest_SOURCES = kxerrorhandlertest.cpp
startserviceby_SOURCES = startserviceby.cpp
kglobaltest_SOURCES = kglobaltest.cpp
ktimezonestest_SOURCES = ktimezonestest.cpp

# Automated unit tests
ktempfiletest_SOURCES = ktempfiletest.cpp
kurlmimetest_SOURCES = kurlmimetest.cpp
kconfigtest_SOURCES = kconfigtest.cpp
krfcdatetest_SOURCES = krfcdatetest.cpp
kstdacceltest_SOURCES = kstdacceltest.cpp
kstringhandlertest_SOURCES = kstringhandlertest.cpp
kurltest4_SOURCES = kurltest4.cpp kurl4.cpp
kcalendartest_SOURCES = kcalendartest.cpp
klibloadertest_SOURCES = klibloadertest.cpp
kcompletiontest_SOURCES = kcompletiontest.cpp
klockfiletest_SOURCES = klockfiletest.cpp
ktempdirtest_SOURCES = ktempdirtest.cpp
ksharedptrtest_SOURCES = ksharedptrtest.cpp
kfiltertest_SOURCES = kfiltertest.cpp
kdatetimetest_SOURCES = kdatetimetest.cpp
ksavefiletest_SOURCES = ksavefiletest.cpp
