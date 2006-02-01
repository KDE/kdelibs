kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfstest_SRCS
kfstest.cpp
kfdtest.cpp
)

kde4_automoc(${kfstest_SRCS})

kde4_add_executable(kfstest ${kfstest_SRCS})

target_link_libraries(kfstest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(kurlrequestertest_SRCS
kurlrequestertest.cpp
)

kde4_automoc(${kurlrequestertest_SRCS})

kde4_add_executable(kurlrequestertest ${kurlrequestertest_SRCS})

target_link_libraries(kurlrequestertest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(kfiletreeviewtest_SRCS
kfiletreeviewtest.cpp
)

kde4_automoc(${kfiletreeviewtest_SRCS})

kde4_add_executable(kfiletreeviewtest ${kfiletreeviewtest_SRCS})

target_link_libraries(kfiletreeviewtest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(kopenwithtest_SRCS
kopenwithtest.cpp
)

kde4_automoc(${kopenwithtest_SRCS})

kde4_add_executable(kopenwithtest ${kopenwithtest_SRCS})

target_link_libraries(kopenwithtest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(kdirselectdialogtest_SRCS
kdirselectdialogtest.cpp
)

kde4_automoc(${kdirselectdialogtest_SRCS})

kde4_add_executable(kdirselectdialogtest ${kdirselectdialogtest_SRCS})

target_link_libraries(kdirselectdialogtest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(kicondialogtest_SRCS
kicondialogtest.cpp
)

kde4_automoc(${kicondialogtest_SRCS})

kde4_add_executable(kicondialogtest ${kicondialogtest_SRCS})

target_link_libraries(kicondialogtest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(knotifytest_SRCS
knotifytest.cpp
)

kde4_automoc(${knotifytest_SRCS})

kde4_add_executable(knotifytest ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(kcustommenueditortest_SRCS
kcustommenueditortest.cpp
)

kde4_automoc(${kcustommenueditortest_SRCS})

kde4_add_executable(kcustommenueditortest ${kcustommenueditortest_SRCS})

target_link_libraries(kcustommenueditortest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

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
#INCLUDES = $(all_includes)
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#
#check_PROGRAMS = kfstest kurlrequestertest kfiletreeviewtest \
#                 kopenwithtest kdirselectdialogtest kicondialogtest \
#                 knotifytest kcustommenueditortest
#
## noinst_HEADERS = 
#
#METASOURCES = AUTO
#
#LDADD = $(LIB_KIO)
#kcustommenueditortest_SOURCES = kcustommenueditortest.cpp
#kurlrequestertest_SOURCES = kurlrequestertest.cpp
#kfstest_SOURCES = kfstest.cpp kfdtest.cpp
#kfiletreeviewtest_SOURCES = kfiletreeviewtest.cpp
#kopenwithtest_SOURCES = kopenwithtest.cpp
#kdirselectdialogtest_SOURCES = kdirselectdialogtest.cpp
#kicondialogtest_SOURCES = kicondialogtest.cpp
#knotifytest_SOURCES = knotifytest.cpp
#
