kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${CMAKE_SOURCE_DIR}/kdecore ${CMAKE_SOURCE_DIR}/kdeui ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/kde3support/kdeui)


########### next target ###############

set(kdockwidgetdemo_SRCS
kdockwidgetdemo.cpp
)

kde4_automoc(${kdockwidgetdemo_SRCS})

kde4_add_executable(kdockwidgetdemo ${kdockwidgetdemo_SRCS})

target_link_libraries(kdockwidgetdemo  ${QT_AND_KDECORE_LIBS} kde3support)


########### next target ###############

set(kdockwidgettest_SRCS
kdockwidgettest.cpp
)

kde4_automoc(${kdockwidgettest_SRCS})

kde4_add_executable(kdockwidgettest ${kdockwidgettest_SRCS})

target_link_libraries(kdockwidgettest  ${QT_AND_KDECORE_LIBS} kde3support)


########### next target ###############

set(kdocktest_SRCS
kdocktest.cpp
)

kde4_automoc(${kdocktest_SRCS})

kde4_add_executable(kdocktest ${kdocktest_SRCS})

target_link_libraries(kdocktest  ${QT_AND_KDECORE_LIBS} kde3support)

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##	This file is part of the KDE libraries
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
#INCLUDES = -I$(top_srcdir)/kdefx -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui -I$(srcdir)/../kdeui \
#	  $(all_includes)
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#
#check_PROGRAMS = kdockwidgetdemo kdockwidgettest kdocktest
#
#METASOURCES = AUTO
#
#LDADD = ../libkde3support.la
#
#kdockwidgetdemo_SOURCES = kdockwidgetdemo.cpp
#kdockwidgettest_SOURCES = kdockwidgettest.cpp
#kdocktest_SOURCES = kdocktest.cpp
