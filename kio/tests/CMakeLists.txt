kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/kio/bookmarks ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(ksycocatest_SRCS
ksycocatest.cpp
)

kde4_automoc(${ksycocatest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ksycocatest ${ksycocatest_SRCS})

target_link_libraries(ksycocatest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(getalltest_SRCS
getalltest.cpp
)

kde4_automoc(${getalltest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(getalltest ${getalltest_SRCS})

target_link_libraries(getalltest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kruntest_SRCS
kruntest.cpp
)

kde4_automoc(${kruntest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kruntest ${kruntest_SRCS})

target_link_libraries(kruntest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ktartest_SRCS
ktartest.cpp
)

kde4_automoc(${ktartest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ktartest ${ktartest_SRCS})

target_link_libraries(ktartest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kziptest_SRCS
kziptest.cpp
)

kde4_automoc(${kziptest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kziptest ${kziptest_SRCS})

target_link_libraries(kziptest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kioslavetest_SRCS
kioslavetest.cpp
)

kde4_automoc(${kioslavetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kioslavetest ${kioslavetest_SRCS})

target_link_libraries(kioslavetest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdirwatchtest_SRCS
kdirwatchtest.cpp
)

kde4_automoc(${kdirwatchtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kdirwatchtest ${kdirwatchtest_SRCS})

target_link_libraries(kdirwatchtest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(speed_SRCS
speed.cpp
)

kde4_automoc(${speed_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(speed ${speed_SRCS})

target_link_libraries(speed  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kurifiltertest_SRCS
kurifiltertest.cpp
)

kde4_automoc(${kurifiltertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kurifiltertest ${kurifiltertest_SRCS})

target_link_libraries(kurifiltertest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdefaultprogresstest_SRCS
kdefaultprogresstest.cpp
)

kde4_automoc(${kdefaultprogresstest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kdefaultprogresstest ${kdefaultprogresstest_SRCS})

target_link_libraries(kdefaultprogresstest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmimemagictest_SRCS
kmimemagictest.cpp
)

kde4_automoc(${kmimemagictest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kmimemagictest ${kmimemagictest_SRCS})

target_link_libraries(kmimemagictest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kiopassdlgtest_SRCS
kiopassdlgtest.cpp
)

kde4_automoc(${kiopassdlgtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kiopassdlgtest ${kiopassdlgtest_SRCS})

target_link_libraries(kiopassdlgtest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kscantest_SRCS
kscantest.cpp
)

kde4_automoc(${kscantest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kscantest ${kscantest_SRCS})

target_link_libraries(kscantest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdirlistertest_SRCS
kdirlistertest.cpp
)

kde4_automoc(${kdirlistertest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kdirlistertest ${kdirlistertest_SRCS})

target_link_libraries(kdirlistertest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(previewtest_SRCS
previewtest.cpp
)

kde4_automoc(${previewtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(previewtest ${previewtest_SRCS})

target_link_libraries(previewtest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kionetrctest_SRCS
kionetrctest.cpp
)

kde4_automoc(${kionetrctest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kionetrctest ${kionetrctest_SRCS})

target_link_libraries(kionetrctest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdcopcheck_SRCS
kdcopcheck.cpp
)

kde4_automoc(${kdcopcheck_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kdcopcheck ${kdcopcheck_SRCS})

target_link_libraries(kdcopcheck  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(metatest_SRCS
metatest.cpp
)

kde4_automoc(${metatest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(metatest ${metatest_SRCS})

target_link_libraries(metatest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmimefromext_SRCS
kmimefromext.cpp
)

kde4_automoc(${kmimefromext_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kmimefromext ${kmimefromext_SRCS})

target_link_libraries(kmimefromext  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kpropsdlgtest_SRCS
kpropsdlgtest.cpp
)

kde4_automoc(${kpropsdlgtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kpropsdlgtest ${kpropsdlgtest_SRCS})

target_link_libraries(kpropsdlgtest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmfitest_SRCS
kmfitest.cpp
)

kde4_automoc(${kmfitest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kmfitest ${kmfitest_SRCS})

target_link_libraries(kmfitest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(dataprotocoltest_SRCS
dataprotocoltest.cpp
)

kde4_automoc(${dataprotocoltest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(dataprotocoltest ${dataprotocoltest_SRCS})

target_link_libraries(dataprotocoltest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kprotocolinfotest_SRCS
kprotocolinfotest.cpp
)

kde4_automoc(${kprotocolinfotest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kprotocolinfotest ${kprotocolinfotest_SRCS})

target_link_libraries(kprotocolinfotest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(ksycocaupdatetest_SRCS
ksycocaupdatetest.cpp
)

kde4_automoc(${ksycocaupdatetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(ksycocaupdatetest ${ksycocaupdatetest_SRCS})

target_link_libraries(ksycocaupdatetest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(netaccesstest_SRCS
netaccesstest.cpp
)

kde4_automoc(${netaccesstest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(netaccesstest ${netaccesstest_SRCS})

target_link_libraries(netaccesstest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(jobtest_SRCS
jobtest.cpp
)

kde4_automoc(${jobtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(jobtest ${jobtest_SRCS})

target_link_libraries(jobtest  ${QT_AND_KDECORE_LIBS} kio ${QT_QTTEST_LIBRARY} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kurlcompletiontest_SRCS
kurlcompletiontest.cpp
)

kde4_automoc(${kurlcompletiontest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kurlcompletiontest ${kurlcompletiontest_SRCS})

target_link_libraries(kurlcompletiontest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kmimetypetest_SRCS
kmimetypetest.cpp
)

kde4_automoc(${kmimetypetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kmimetypetest ${kmimetypetest_SRCS})

target_link_libraries(kmimetypetest  ${QT_AND_KDECORE_LIBS} kio ${QT_QTTEST_LIBRARY} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kbookmarktest_SRCS
kbookmarktest.cpp
)

kde4_automoc(${kbookmarktest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kbookmarktest ${kbookmarktest_SRCS})

target_link_libraries(kbookmarktest  ${QT_AND_KDECORE_LIBS} kio ${QT_QTTEST_LIBRARY} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kacltest_SRCS
kacltest.cpp
)

kde4_automoc(${kacltest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kacltest ${kacltest_SRCS})

target_link_libraries(kacltest  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kdirwatchtest_gui_SRCS
kdirwatchtest_gui.cpp
)

kde4_automoc(${kdirwatchtest_gui_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kdirwatchtest_gui ${kdirwatchtest_gui_SRCS})

target_link_libraries(kdirwatchtest_gui  ${QT_AND_KDECORE_LIBS} kio)

endif(KDE4_BUILD_TESTS)

########### next target ###############

if(KDE4_BUILD_TESTS)

set(ktradertest_SRCS
ktradertest.cpp
)

kde4_automoc(${ktradertest_SRCS})

kde4_add_executable(ktradertest ${ktradertest_SRCS})

target_link_libraries(ktradertest  ${QT_AND_KDECORE_LIBS} kio)

install_targets(/bin ktradertest )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1997 David Faure <faure@kde.org>
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
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kio/bookmarks $(all_includes)
#LDADD = $(LIB_KIO)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#check_PROGRAMS = ksycocatest getalltest kruntest ktartest kziptest\
#                 kioslavetest kdirwatchtest speed kurifiltertest \
#                 kdefaultprogresstest kmimemagictest \
#                 kiopassdlgtest kscantest kdirlistertest \
#                 previewtest kionetrctest kdcopcheck metatest \
#		 kmimefromext kpropsdlgtest kmfitest dataprotocoltest \
#		 kprotocolinfotest ksycocaupdatetest netaccesstest jobtest \
#		 kurlcompletiontest kmimetypetest kbookmarktest kacltest kdirwatchtest_gui
#
## Unfortunately some tests depend on the network settings, it seems
##check: kurifiltertest
##	./kurifiltertest
#check-local: kziptest
#	./kziptest list $(srcdir)/wronglocalsizes.zip
#
#TESTS = kmimetypetest kbookmarktest
#
#bin_PROGRAMS = ktradertest
#
#METASOURCES = AUTO
#
#speed_SOURCES = speed.cpp
#kioslavetest_SOURCES = kioslavetest.cpp
#kshredtest_SOURCES = kshredtest.cpp
#kdefaultprogresstest_SOURCES = kdefaultprogresstest.cpp
#kionetrctest_SOURCES = kionetrctest.cpp
#kiopassdlgtest_SOURCES = kiopassdlgtest.cpp
#kurifiltertest_SOURCES = kurifiltertest.cpp
#ksycocatest_SOURCES = ksycocatest.cpp
#kdcopcheck_SOURCES = kdcopcheck.cpp
#getalltest_SOURCES = getalltest.cpp
#kruntest_SOURCES = kruntest.cpp
#kdirwatchtest_SOURCES = kdirwatchtest.cpp
#ktradertest_SOURCES = ktradertest.cpp
#kmimemagictest_SOURCES = kmimemagictest.cpp
#kscantest_SOURCES = kscantest.cpp
#kdirlistertest_SOURCES = kdirlistertest.cpp
#previewtest_SOURCES = previewtest.cpp
#ktartest_SOURCES = ktartest.cpp
#kziptest_SOURCES = kziptest.cpp
#metatest_SOURCES = metatest.cpp
#kmimefromext_SOURCES = kmimefromext.cpp
#kpropsdlgtest_SOURCES = kpropsdlgtest.cpp
#kmfitest_SOURCES = kmfitest.cpp
#dataprotocoltest_SOURCES = dataprotocoltest.cpp
#kprotocolinfotest_SOURCES = kprotocolinfotest.cpp
#ksycocaupdatetest_SOURCES = ksycocaupdatetest.cpp
#netaccesstest_SOURCES = netaccesstest.cpp
#kurlcompletiontest_SOURCES = kurlcompletiontest.cpp
#kacltest_SOURCES = kacltest.cpp
#kdirwatchtest_gui_SOURCES = kdirwatchtest_gui.cpp
#
## kfile meta stuff. Comment this in, if you want a small
## metadata plugin test and "make install".
##kde_module_LTLIBRARIES = dummymeta.la
##dummymeta_la_SOURCES = dummymeta.cpp
##dummymeta_la_LIBADD = $(LIB_KIO)
##dummymeta_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
##services_DATA = dummymeta.desktop
##servicesdir = $(kde_servicesdir)
#
#kmimetypetest_SOURCES = kmimetypetest.cpp
#kmimetypetest_LDADD = $(LIB_KIO) -l${QT_QTTEST_LIBRARY}
#
#kbookmarktest_SOURCES = kbookmarktest.cpp
#kbookmarktest_LDADD = $(LIB_KIO) -l${QT_QTTEST_LIBRARY}
#
#jobtest_SOURCES = jobtest.cpp
#jobtest_LDADD = $(LIB_KIO) -l${QT_QTTEST_LIBRARY}
