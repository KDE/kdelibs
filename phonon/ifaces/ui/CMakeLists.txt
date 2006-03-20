kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(phononifacesui_LIB_SRCS backend.cpp )

kde4_automoc(${phononifacesui_LIB_SRCS})

kde4_add_library(phononifacesui SHARED ${phononifacesui_LIB_SRCS})

target_link_libraries(phononifacesui  ${QT_AND_KDECORE_LIBS} phononifaces )

set_target_properties(phononifacesui PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} phononifacesui )


########### install files ###############

install_files( /include/phonon/ifaces/ui FILES  videowidget.h backend.h )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License version 2 as published by the Free Software Foundation.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
##    Boston, MA 02111-1307, USA.
#
#INCLUDES = -I$(top_srcdir) $(all_includes)
#
#SUBDIRS = .
#
#lib_LTLIBRARIES = libphononifacesui.la
#
#phononincludedir = $(includedir)/phonon/ifaces/ui
#phononinclude_HEADERS = videowidget.h backend.h
#
#libphononifacesui_la_SOURCES = backend.cpp
#libphononifacesui_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_LDFLAGS) $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
#libphononifacesui_la_LIBADD = ../libphononifaces.la $(LIB_KDECORE)
#
#METASOURCES = AUTO
