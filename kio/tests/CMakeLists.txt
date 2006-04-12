kde4_header()

# for <kio/global.h>
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########## ktradertest - which is installed

kde4_add_executable(ktradertest NOGUI ktradertest.cpp)

target_link_libraries(ktradertest ${KDE4_KIO_LIBS})

install_targets(/bin ktradertest )

if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(ksycocatest_SRCS ksycocatest.cpp)

kde4_automoc(${ksycocatest_SRCS})

kde4_add_executable(ksycocatest RUN_UNINSTALLED ${ksycocatest_SRCS})

target_link_libraries(ksycocatest ${KDE4_KIO_LIBS})


########### next target ###############

set(getalltest_SRCS getalltest.cpp)

kde4_automoc(${getalltest_SRCS})

kde4_add_executable(getalltest RUN_UNINSTALLED ${getalltest_SRCS})

target_link_libraries(getalltest ${KDE4_KIO_LIBS})


########### next target ###############

set(kruntest_SRCS kruntest.cpp)

kde4_automoc(${kruntest_SRCS})

kde4_add_executable(kruntest RUN_UNINSTALLED ${kruntest_SRCS})

target_link_libraries(kruntest ${KDE4_KIO_LIBS})


########### next target ###############

set(ktartest_SRCS ktartest.cpp)

kde4_automoc(${ktartest_SRCS})

kde4_add_executable(ktartest RUN_UNINSTALLED ${ktartest_SRCS})

target_link_libraries(ktartest ${KDE4_KIO_LIBS})


########### next target ###############

set(kziptest_SRCS kziptest.cpp)

kde4_automoc(${kziptest_SRCS})

kde4_add_executable(kziptest RUN_UNINSTALLED ${kziptest_SRCS})

target_link_libraries(kziptest ${KDE4_KIO_LIBS})

########### next target ###############

set(karchivetest_SRCS karchivetest.cpp)

kde4_automoc(${karchivetest_SRCS})

kde4_add_executable(karchivetest RUN_UNINSTALLED ${karchivetest_SRCS})

target_link_libraries(karchivetest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kio-karchivetest ${EXECUTABLE_OUTPUT_PATH}/karchivetest)

########### next target ###############

set(kziptest_SRCS kziptest.cpp)

kde4_automoc(${kziptest_SRCS})

kde4_add_executable(kziptest RUN_UNINSTALLED ${kziptest_SRCS})

target_link_libraries(kziptest ${KDE4_KIO_LIBS})

########### next target ###############

set(kioslavetest_SRCS kioslavetest.cpp)

kde4_automoc(${kioslavetest_SRCS})

kde4_add_executable(kioslavetest RUN_UNINSTALLED ${kioslavetest_SRCS})

target_link_libraries(kioslavetest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdirwatchtest_SRCS kdirwatchtest.cpp)

kde4_automoc(${kdirwatchtest_SRCS})

kde4_add_executable(kdirwatchtest RUN_UNINSTALLED ${kdirwatchtest_SRCS})

target_link_libraries(kdirwatchtest ${KDE4_KIO_LIBS})


########### next target ###############

set(speed_SRCS speed.cpp)

kde4_automoc(${speed_SRCS})

kde4_add_executable(speed RUN_UNINSTALLED ${speed_SRCS})

target_link_libraries(speed ${KDE4_KIO_LIBS})


########### next target ###############

set(kurifiltertest_SRCS kurifiltertest.cpp)

kde4_automoc(${kurifiltertest_SRCS})

kde4_add_executable(kurifiltertest RUN_UNINSTALLED ${kurifiltertest_SRCS})

target_link_libraries(kurifiltertest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdefaultprogresstest_SRCS kdefaultprogresstest.cpp)

kde4_automoc(${kdefaultprogresstest_SRCS})

kde4_add_executable(kdefaultprogresstest RUN_UNINSTALLED ${kdefaultprogresstest_SRCS})

target_link_libraries(kdefaultprogresstest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmimemagictest_SRCS kmimemagictest.cpp)

kde4_automoc(${kmimemagictest_SRCS})

kde4_add_executable(kmimemagictest RUN_UNINSTALLED ${kmimemagictest_SRCS})

target_link_libraries(kmimemagictest ${KDE4_KIO_LIBS})


########### next target ###############

set(kiopassdlgtest_SRCS kiopassdlgtest.cpp)

kde4_automoc(${kiopassdlgtest_SRCS})

kde4_add_executable(kiopassdlgtest RUN_UNINSTALLED ${kiopassdlgtest_SRCS})

target_link_libraries(kiopassdlgtest ${KDE4_KIO_LIBS})


########### next target ###############

set(kscantest_SRCS kscantest.cpp)

kde4_automoc(${kscantest_SRCS})

kde4_add_executable(kscantest RUN_UNINSTALLED ${kscantest_SRCS})

target_link_libraries(kscantest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdirlistertest_SRCS kdirlistertest.cpp)

kde4_automoc(${kdirlistertest_SRCS})

kde4_add_executable(kdirlistertest RUN_UNINSTALLED ${kdirlistertest_SRCS})

target_link_libraries(kdirlistertest ${KDE4_KIO_LIBS})


########### next target ###############

set(previewtest_SRCS previewtest.cpp)

kde4_automoc(${previewtest_SRCS})

kde4_add_executable(previewtest RUN_UNINSTALLED ${previewtest_SRCS})

target_link_libraries(previewtest ${KDE4_KIO_LIBS})


########### next target ###############

set(kionetrctest_SRCS kionetrctest.cpp)

kde4_automoc(${kionetrctest_SRCS})

kde4_add_executable(kionetrctest RUN_UNINSTALLED ${kionetrctest_SRCS})

target_link_libraries(kionetrctest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdcopcheck_SRCS kdcopcheck.cpp)

kde4_automoc(${kdcopcheck_SRCS})

kde4_add_executable(kdcopcheck RUN_UNINSTALLED ${kdcopcheck_SRCS})

target_link_libraries(kdcopcheck ${KDE4_KIO_LIBS})


########### next target ###############

set(metatest_SRCS metatest.cpp)

kde4_automoc(${metatest_SRCS})

kde4_add_executable(metatest RUN_UNINSTALLED ${metatest_SRCS})

target_link_libraries(metatest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmimefromext_SRCS kmimefromext.cpp)

kde4_automoc(${kmimefromext_SRCS})

kde4_add_executable(kmimefromext RUN_UNINSTALLED ${kmimefromext_SRCS})

target_link_libraries(kmimefromext ${KDE4_KIO_LIBS})


########### next target ###############

set(kpropsdlgtest_SRCS kpropsdlgtest.cpp)

kde4_automoc(${kpropsdlgtest_SRCS})

kde4_add_executable(kpropsdlgtest RUN_UNINSTALLED ${kpropsdlgtest_SRCS})

target_link_libraries(kpropsdlgtest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmfitest_SRCS kmfitest.cpp)

kde4_automoc(${kmfitest_SRCS})

kde4_add_executable(kmfitest RUN_UNINSTALLED ${kmfitest_SRCS})

target_link_libraries(kmfitest ${KDE4_KIO_LIBS})


########### next target ###############

set(dataprotocoltest_SRCS dataprotocoltest.cpp)

kde4_automoc(${dataprotocoltest_SRCS})

kde4_add_executable(dataprotocoltest RUN_UNINSTALLED ${dataprotocoltest_SRCS})

target_link_libraries(dataprotocoltest ${KDE4_KIO_LIBS})


########### next target ###############

set(kprotocolinfotest_SRCS kprotocolinfotest.cpp)

kde4_automoc(${kprotocolinfotest_SRCS})
kde4_add_executable(kprotocolinfotest RUN_UNINSTALLED ${kprotocolinfotest_SRCS})

target_link_libraries(kprotocolinfotest ${KDE4_KIO_LIBS})


########### next target ###############

set(ksycocaupdatetest_SRCS ksycocaupdatetest.cpp)

kde4_automoc(${ksycocaupdatetest_SRCS})

kde4_add_executable(ksycocaupdatetest RUN_UNINSTALLED ${ksycocaupdatetest_SRCS})

target_link_libraries(ksycocaupdatetest ${KDE4_KIO_LIBS})


########### next target ###############

set(netaccesstest_SRCS netaccesstest.cpp)

kde4_automoc(${netaccesstest_SRCS})

kde4_add_executable(netaccesstest RUN_UNINSTALLED ${netaccesstest_SRCS})

target_link_libraries(netaccesstest ${KDE4_KIO_LIBS})


########### next target ###############

set(jobtest_SRCS jobtest.cpp)

kde4_automoc(${jobtest_SRCS})

kde4_add_executable(jobtest RUN_UNINSTALLED ${jobtest_SRCS})

target_link_libraries(jobtest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} )


########### next target ###############

set(kurlcompletiontest_SRCS kurlcompletiontest.cpp)

kde4_automoc(${kurlcompletiontest_SRCS})

kde4_add_executable(kurlcompletiontest RUN_UNINSTALLED ${kurlcompletiontest_SRCS})

target_link_libraries(kurlcompletiontest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmimetypetest_SRCS kmimetypetest.cpp)

kde4_automoc(${kmimetypetest_SRCS})

kde4_add_executable(kmimetypetest RUN_UNINSTALLED ${kmimetypetest_SRCS})

target_link_libraries(kmimetypetest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} )

add_test(kio-kmimetypetest ${EXECUTABLE_OUTPUT_PATH}/kmimetypetest)


########### next target ###############

set(kbookmarktest_SRCS kbookmarktest.cpp)

kde4_automoc(${kbookmarktest_SRCS})

kde4_add_executable(kbookmarktest RUN_UNINSTALLED ${kbookmarktest_SRCS})

target_link_libraries(kbookmarktest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} )

add_test(kio-kbookmarktest ${EXECUTABLE_OUTPUT_PATH}/kbookmarktest)

########### next target ###############

set(kacltest_SRCS kacltest.cpp)

kde4_automoc(${kacltest_SRCS})

kde4_add_executable(kacltest RUN_UNINSTALLED ${kacltest_SRCS})

target_link_libraries(kacltest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdirwatchtest_gui_SRCS kdirwatchtest_gui.cpp)

kde4_automoc(${kdirwatchtest_gui_SRCS})

kde4_add_executable(kdirwatchtest_gui RUN_UNINSTALLED ${kdirwatchtest_gui_SRCS})

target_link_libraries(kdirwatchtest_gui ${KDE4_KIO_LIBS})


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
