kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdecore ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kconfigtestgui_SRCS
kconfigtestgui.cpp
)

kde4_automoc(${kconfigtestgui_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kconfigtestgui ${kconfigtestgui_SRCS})

target_link_libraries(kconfigtestgui  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(klocaletest_SRCS
klocaletest.cpp
)

kde4_automoc(${klocaletest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(klocaletest ${klocaletest_SRCS})

target_link_libraries(klocaletest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kprocesstest_SRCS
kprocesstest.cpp
)

kde4_automoc(${kprocesstest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kprocesstest ${kprocesstest_SRCS})

target_link_libraries(kprocesstest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ksimpleconfigtest_SRCS
ksimpleconfigtest.cpp
)

kde4_automoc(${ksimpleconfigtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ksimpleconfigtest ${ksimpleconfigtest_SRCS})

target_link_libraries(ksimpleconfigtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kstddirstest_SRCS
kstddirstest.cpp
)

kde4_automoc(${kstddirstest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kstddirstest ${kstddirstest_SRCS})

target_link_libraries(kstddirstest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kurltest_SRCS
kurltest.cpp
)

kde4_automoc(${kurltest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kurltest ${kurltest_SRCS})

target_link_libraries(kurltest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kurltest4_SRCS
kurltest4.cpp
kurl4.cpp
)

kde4_automoc(${kurltest4_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kurltest4 ${kurltest4_SRCS})

target_link_libraries(kurltest4  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kuniqueapptest_SRCS
kuniqueapptest.cpp
)

kde4_automoc(${kuniqueapptest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kuniqueapptest ${kuniqueapptest_SRCS})

target_link_libraries(kuniqueapptest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(krandomsequencetest_SRCS
krandomsequencetest.cpp
)

kde4_automoc(${krandomsequencetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(krandomsequencetest ${krandomsequencetest_SRCS})

target_link_libraries(krandomsequencetest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdebugtest_SRCS
kdebugtest.cpp
)

kde4_automoc(${kdebugtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kdebugtest ${kdebugtest_SRCS})

target_link_libraries(kdebugtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kstringhandlertest_SRCS
kstringhandlertest.cpp
)

kde4_automoc(${kstringhandlertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kstringhandlertest ${kstringhandlertest_SRCS})

target_link_libraries(kstringhandlertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kcmdlineargstest_SRCS
kcmdlineargstest.cpp
)

kde4_automoc(${kcmdlineargstest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kcmdlineargstest ${kcmdlineargstest_SRCS})

target_link_libraries(kcmdlineargstest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kapptest_SRCS
kapptest.cpp
)

kde4_automoc(${kapptest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kapptest ${kapptest_SRCS})

target_link_libraries(kapptest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmemtest_SRCS
kmemtest.cpp
)

kde4_automoc(${kmemtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kmemtest ${kmemtest_SRCS})

target_link_libraries(kmemtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kidlservertest_SRCS
KIDLTest.cpp
)

kde4_automoc(${kidlservertest_SRCS})

set( kidlservertest_DCOP_SKEL_SRCS
KIDLTest.h
)

kde4_add_dcop_skels(kidlservertest_SRCS ${kidlservertest_DCOP_SKEL_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kidlservertest ${kidlservertest_SRCS})

target_link_libraries(kidlservertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kidlclienttest_SRCS
KIDLTestClient.cpp
)

kde4_automoc(${kidlclienttest_SRCS})

set( kidlclienttest_DCOP_STUB_SRCS
KIDLTest.h
)

kde4_add_dcop_stubs(kidlclienttest_SRCS ${kidlclienttest_DCOP_STUB_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kidlclienttest ${kidlclienttest_SRCS})

target_link_libraries(kidlclienttest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(dcopkonqtest_SRCS
dcopkonqtest.cpp
)

kde4_automoc(${dcopkonqtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(dcopkonqtest ${dcopkonqtest_SRCS})

target_link_libraries(dcopkonqtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kipctest_SRCS
kipctest.cpp
)

kde4_automoc(${kipctest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kipctest ${kipctest_SRCS})

target_link_libraries(kipctest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(cplusplustest_SRCS
cplusplustest.cpp
)

kde4_automoc(${cplusplustest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(cplusplustest ${cplusplustest_SRCS})

target_link_libraries(cplusplustest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kiconloadertest_SRCS
kiconloadertest.cpp
)

kde4_automoc(${kiconloadertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kiconloadertest ${kiconloadertest_SRCS})

target_link_libraries(kiconloadertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmdcodectest_SRCS
kmdcodectest.cpp
)

kde4_automoc(${kmdcodectest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kmdcodectest ${kmdcodectest_SRCS})

target_link_libraries(kmdcodectest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(knotifytest_SRCS
knotifytest.cpp
)

kde4_automoc(${knotifytest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(knotifytest ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdatetimetest_SRCS
kdatetimetest.cpp
)

kde4_automoc(${kdatetimetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ksortablevaluelisttest_SRCS
ksortablevaluelisttest.cpp
)

kde4_automoc(${ksortablevaluelisttest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ksortablevaluelisttest ${ksortablevaluelisttest_SRCS})

target_link_libraries(ksortablevaluelisttest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(krfcdatetest_SRCS
krfcdatetest.cpp
)

kde4_automoc(${krfcdatetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(krfcdatetest ${krfcdatetest_SRCS})

target_link_libraries(krfcdatetest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testqtargs_SRCS
testqtargs.cpp
)

kde4_automoc(${testqtargs_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testqtargs ${testqtargs_SRCS})

target_link_libraries(testqtargs  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kprociotest_SRCS
kprociotest.cpp
)

kde4_automoc(${kprociotest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kprociotest ${kprociotest_SRCS})

target_link_libraries(kprociotest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kcharsetstest_SRCS
kcharsetstest.cpp
)

kde4_automoc(${kcharsetstest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kcharsetstest ${kcharsetstest_SRCS})

target_link_libraries(kcharsetstest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kcalendartest_SRCS
kcalendartest.cpp
)

kde4_automoc(${kcalendartest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kcalendartest ${kcalendartest_SRCS})

target_link_libraries(kcalendartest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmacroexpandertest_SRCS
kmacroexpandertest.cpp
)

kde4_automoc(${kmacroexpandertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kmacroexpandertest ${kmacroexpandertest_SRCS})

target_link_libraries(kmacroexpandertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kshelltest_SRCS
kshelltest.cpp
)

kde4_automoc(${kshelltest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kshelltest ${kshelltest_SRCS})

target_link_libraries(kshelltest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kxerrorhandlertest_SRCS
kxerrorhandlertest.cpp
)

kde4_automoc(${kxerrorhandlertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kxerrorhandlertest ${kxerrorhandlertest_SRCS})

target_link_libraries(kxerrorhandlertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(startserviceby_SRCS
startserviceby.cpp
)

kde4_automoc(${startserviceby_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(startserviceby ${startserviceby_SRCS})

target_link_libraries(startserviceby  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kglobaltest_SRCS
kglobaltest.cpp
)

kde4_automoc(${kglobaltest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kglobaltest ${kglobaltest_SRCS})

target_link_libraries(kglobaltest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ktimezonestest_SRCS
ktimezonestest.cpp
)

kde4_automoc(${ktimezonestest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ktimezonestest ${ktimezonestest_SRCS})

target_link_libraries(ktimezonestest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kconfigtest_SRCS
kconfigtest.cpp
)

kde4_automoc(${kconfigtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kconfigtest ${kconfigtest_SRCS})

target_link_libraries(kconfigtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ktempfiletest_SRCS
ktempfiletest.cpp
)

kde4_automoc(${ktempfiletest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ktempfiletest ${ktempfiletest_SRCS})

target_link_libraries(ktempfiletest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kurlmimetest_SRCS
kurlmimetest.cpp
)

kde4_automoc(${kurlmimetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kurlmimetest ${kurlmimetest_SRCS})

target_link_libraries(kurlmimetest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kstdacceltest_SRCS
kstdacceltest.cpp
)

kde4_automoc(${kstdacceltest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kstdacceltest ${kstdacceltest_SRCS})

target_link_libraries(kstdacceltest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(klibloadertest_SRCS
klibloadertest.cpp
)

kde4_automoc(${klibloadertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(klibloadertest ${klibloadertest_SRCS})

target_link_libraries(klibloadertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kcompletiontest_SRCS
kcompletiontest.cpp
)

kde4_automoc(${kcompletiontest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kcompletiontest ${kcompletiontest_SRCS})

target_link_libraries(kcompletiontest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(klockfiletest_SRCS
klockfiletest.cpp
)

kde4_automoc(${klockfiletest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(klockfiletest ${klockfiletest_SRCS})

target_link_libraries(klockfiletest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ktempdirtest_SRCS
ktempdirtest.cpp
)

kde4_automoc(${ktempdirtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ktempdirtest ${ktempdirtest_SRCS})

target_link_libraries(ktempdirtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ksharedptrtest_SRCS
ksharedptrtest.cpp
)

kde4_automoc(${ksharedptrtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ksharedptrtest ${ksharedptrtest_SRCS})

target_link_libraries(ksharedptrtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kfiltertest_SRCS
kfiltertest.cpp
)

kde4_automoc(${kfiltertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kfiltertest ${kfiltertest_SRCS})

target_link_libraries(kfiltertest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdatetimetest_SRCS
dummy.cpp
)

kde4_automoc(${kdatetimetest_SRCS})

file(WRITE dummy.cpp "//autogenerated file by cmake\n")
if(KDE4_BUILD_TESTS)
kde4_add_executable(kdatetimetest ${kdatetimetest_SRCS})

target_link_libraries(kdatetimetest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(klibloadertestmodule_PART_SRCS
klibloadertest_module.cpp
)

kde4_automoc(${klibloadertestmodule_PART_SRCS})

kde4_add_plugin(klibloadertestmodule WITH_PREFIX ${klibloadertestmodule_PART_SRCS})

target_link_libraries(klibloadertestmodule  ${QT_AND_KDECORE_LIBS} )

install_targets(/lib/kde4 klibloadertestmodule )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
##              (C) 1997-1998 Stephan Kulow (coolo@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#INCLUDES = -I$(top_srcdir)/kdecore $(all_includes)
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#
#check_PROGRAMS = kconfigtestgui klocaletest kprocesstest ksimpleconfigtest \
#	kstddirstest kurltest kurltest4 kuniqueapptest krandomsequencetest \
#	kdebugtest kstringhandlertest kcmdlineargstest kapptest \
#	kmemtest kidlservertest kidlclienttest dcopkonqtest kipctest \
#	cplusplustest kiconloadertest kmdcodectest knotifytest kdatetimetest \
#	ksortablevaluelisttest krfcdatetest testqtargs kprociotest \
#	kcharsetstest kcalendartest kmacroexpandertest kshelltest \
#	kxerrorhandlertest startserviceby kglobaltest ktimezonestest \
#	kconfigtest ktempfiletest kurlmimetest kstdacceltest klibloadertest \
#	kcompletiontest klockfiletest ktempdirtest ksharedptrtest kfiltertest \
#	kdatetimetest
#
## TODO kurltest4 when it works
#
## List of tests to run automatically from "make check"
#TESTS = kurltest kcompletiontest\
#	kconfigtest ktempfiletest kurlmimetest kstdacceltest \
#	kstringhandlertest krfcdatetest kcalendartest klibloadertest
#
## Test module for klibloadertest. Ideally we shouldn't install it.
#kde_module_LTLIBRARIES = libklibloadertestmodule.la
#libklibloadertestmodule_la_SOURCES = klibloadertest_module.cpp
#klibloadertest: libklibloadertestmodule.la
#
#METASOURCES = AUTO
#
#LDADD = ../libkdecore.la -lQtTest_debug
#
## Interactive test programs
#kconfigtestgui_SOURCES = kconfigtestgui.cpp
#kdebugtest_SOURCES = kdebugtest.cpp
#klocaletest_SOURCES = klocaletest.cpp
##klocaletest2_SOURCES = klocaletest2.cpp klocale.cpp libintl.cpp kcatalog.cpp
##kcatalog_SOURCES = kcatalog.cpp libintl.cpp
#ksimpleconfigtest_SOURCES = ksimpleconfigtest.cpp
#kurltest_SOURCES = kurltest.cpp
#kstddirstest_SOURCES = kstddirstest.cpp
#kprocesstest_SOURCES = kprocesstest.cpp
#kuniqueapptest_SOURCES = kuniqueapptest.cpp
#kapptest_SOURCES = kapptest.cpp
#krandomsequencetest_SOURCES = krandomsequencetest.cpp
#kcmdlineargstest_SOURCES = kcmdlineargstest.cpp
#dcopkonqtest_SOURCES = dcopkonqtest.cpp
#kmemtest_SOURCES = kmemtest.cpp
#kidlservertest_SOURCES = KIDLTest.cpp KIDLTest.skel
#kidlclienttest_SOURCES = KIDLTestClient.cpp KIDLTest.stub
#$(srcdir)/KIDLTestClient.cpp: KIDLTest_stub.h
#kipctest_SOURCES = kipctest.cpp
#cplusplustest_SOURCES = cplusplustest.cpp
#kiconloadertest_SOURCES = kiconloadertest.cpp
#kmdcodectest_SOURCES = kmdcodectest.cpp
#knotifytest_SOURCES = knotifytest.cpp
#ksortablevaluelisttest_SOURCES = ksortablevaluelisttest.cpp
#testqtargs_SOURCES = testqtargs.cpp
#kprociotest_SOURCES = kprociotest.cpp
#kcharsetstest_SOURCES = kcharsetstest.cpp
#kmacroexpandertest_SOURCES = kmacroexpandertest.cpp
#kshelltest_SOURCES = kshelltest.cpp
#kxerrorhandlertest_SOURCES = kxerrorhandlertest.cpp
#startserviceby_SOURCES = startserviceby.cpp
#kglobaltest_SOURCES = kglobaltest.cpp
#ktimezonestest_SOURCES = ktimezonestest.cpp
#
## Automated unit tests
#ktempfiletest_SOURCES = ktempfiletest.cpp
#kurlmimetest_SOURCES = kurlmimetest.cpp
#kconfigtest_SOURCES = kconfigtest.cpp
#krfcdatetest_SOURCES = krfcdatetest.cpp
#kstdacceltest_SOURCES = kstdacceltest.cpp
#kstringhandlertest_SOURCES = kstringhandlertest.cpp
#kurltest4_SOURCES = kurltest4.cpp kurl4.cpp
#kcalendartest_SOURCES = kcalendartest.cpp
#klibloadertest_SOURCES = klibloadertest.cpp
#kcompletiontest_SOURCES = kcompletiontest.cpp
#klockfiletest_SOURCES = klockfiletest.cpp
#ktempdirtest_SOURCES = ktempdirtest.cpp
#ksharedptrtest_SOURCES = ksharedptrtest.cpp
#kfiltertest_SOURCES = kfiltertest.cpp
#kdatetimetest_SOURCES = kdatetimetest.cpp
